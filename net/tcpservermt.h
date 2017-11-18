#ifndef TCP_SERVER_MULTITHREAD_H_
#define TCP_SERVER_MULTITHREAD_H_

#include <sys/syscall.h>
#include <vector>
#include <boost/thread/thread.hpp>
#include "tcpserver.h"
#include "thread.h"

namespace znb
{
    class PipeCaller
    {
    public:
        typedef boost::function<void ()> PipeReadCallback;

        PipeCaller(int pipefd, CEpoll* pe):ev(new CEvSource(pipefd, pe))
        {
            ev->setReadCallback(boost::bind(&PipeCaller::handleRead, this));
        }
        ~PipeCaller()
        {
        }

        void setnewConnectionCallback(PipeReadCallback cb)
        {
            pipeCallback = cb;
        }

        void handleRead()
        {
            pipeCallback();
        }

        void listen()
        {
            Globals::GetEpoll()->netEpollAdd(ev.get(), EPOLL_READABLE);
        }

    private:

        PipeReadCallback pipeCallback;

        boost::scoped_ptr<CEvSource> ev; 

    };

    class TcpServerMt : public TcpServer
    {
    public:
        TcpServerMt(uint32_t port);
        ~TcpServerMt();

    public:
        void newConnection(int sockfd, const InetAddress& peerAddr);

    private:
        class Worker : public Thread
        {
        public:
            Worker(IDataHandlerAware* dataHandler);
            ~Worker();

        public:
            int getLoad();
            void postConn(uint32_t uConnId);
            void handleRead();
            void removeConnection(const TcpConnPtr& conn);

        private:
            void process();
            pid_t __getThreadId()
            {
                return syscall(SYS_gettid);
            }

        private:

            // 工作者线程的Selector
            CEpoll m_pE;

            // 当前维护的连接数
            int m_nNumConn;

            // 用于和主线程通信的管道
            int m_pipefd[2];

            boost::scoped_ptr<PipeCaller> pipecaller;

            boost::shared_ptr<IDataHandlerAware> dataHandler;

            std::tr1::unordered_map<uint32_t, TcpConnPtr> conns;
        };

        static const int WorkerSize = 20;
        typedef std::vector<Worker *> Workers;
        Workers m_workers;
    };
}

#endif // TCP_SERVER_MULTITHREAD_H_
