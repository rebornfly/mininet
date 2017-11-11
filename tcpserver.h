#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/syscall.h>  
#include <boost/thread/thread.hpp>
#include <vector>
#include "event_source.h"
#include "net_conn.h"
#include <tr1/unordered_map>
#include "acceptor.h"
#include "../common/znb_ex.h"
namespace znb
{
    class Socket_Exception : public ZNB_Exception
    {
    public:
        Socket_Exception(const string &sBuffer) : ZNB_Exception(sBuffer){}
        ~Socket_Exception() throw(){}    
    };
    
    class CNetSocketHelp{
        public:
            static void netSetSockReuse(int so)
            {
                int yes = 1;
                if(setsockopt(so, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) != 0)
                {
                    log(Error, "set sock SO_REUSERADDR err:%s", strerror(errno));
                    close(so);
                    throw(Socket_Exception("set sock SO_REUSERADDR error"));    
                }
            }
            static int netCreateSocket(int domain)
            {
                int s = socket(domain, SOCK_STREAM, 0);
                if(s == -1)
                {
                    log(Error, "create sock err:%s", strerror(errno));
                    throw(Socket_Exception("create sock error"));
                }
            
                netSetSockReuse(s);
                netSetNonBlock(s);
                return s;
            }

            static void netListen(int so, void* addr, socklen_t len, int backLog = 20)
            {
                if(bind(so, (struct sockaddr*)addr, len) == -1)
                {
                    log(Error, "bind sock err:%s", strerror(errno));
                    close(so);
                    throw(Socket_Exception("bind sock error"));
                }
                if(listen(so, backLog) != 0)
                {
                    log(Error, "listen sock err:%s", strerror(errno));
                    close(so);
                    throw(Socket_Exception("listen error"));
                }
            }


            static void netSetNonBlock(int so)
            {
                int flag  = fcntl(so, F_GETFL);
                if(flag == -1)
                {
                    log(Error, "fcntl(F_GETFL) error:%s", strerror(errno));
                    close(so);
                    throw(Socket_Exception("fcntl(F_GETFL) error"));
                }
                if(fcntl(so, F_SETFL, flag | O_NONBLOCK) == -1)
                {
                    log(Error, "fcntl(F_SETFL) error:%s", strerror(errno));
                    close(so);
                    throw(Socket_Exception("fcntl(F_SETFL) error"));
                }
            }
            
            static int initListenSockfd(uint32_t port, uint32_t ip = INADDR_ANY)
            {
                 int m_fd = CNetSocketHelp::netCreateSocket(AF_INET);
                        struct sockaddr_in sa;
                        sa.sin_family = AF_INET;
                     sa.sin_addr.s_addr = htonl(ip);
                     sa.sin_port = htons(port);
                        
                 CNetSocketHelp::netListen(m_fd, (struct sockaddr*)&sa, sizeof(sa));
                        
                 return m_fd;                
            }

            static CTcpConn* initClientSock(const char* ip, uint32_t uport, CEpoll* pE,  IDataHandler* dataHandler)
            {
                int m_so = ::socket(AF_INET, SOCK_STREAM, 0);
                if(m_so == -1)
                {
                    log(Error, "Create server Error:%s", ip);
                    throw(Socket_Exception("initClientSock, create sock error"));
                }

                struct sockaddr_in sa;
                u_long addr = INADDR_ANY;
                if(ip != NULL)
                {
                    addr = ::inet_addr(ip);
                    if(addr == INADDR_ANY)
                    {
                        close(m_so);
                        log(Error, "addr is NULL,");
                        return NULL;
                    }
                }

                sa.sin_family = AF_INET;
                sa.sin_port = htons(uport);
                sa.sin_addr.s_addr = addr;
                
                int rc = ::connect(m_so, (struct sockaddr*)&sa, sizeof(sa));
                if(rc < 0)
                {
                    close(m_so);
                    log(Error, "Client connet err:%s", strerror(errno));
                    throw(Socket_Exception("connect error"));
                }

                CTcpConn* pConn = new CTcpConn(pE, m_so, inet_ntoa(sa.sin_addr), uport,  dataHandler);

            /*    pConn->setEventType(ENUM_TYPE_CONN);

                pConn->setConnStat(ENUM_STATE_CONNECTED);

                pConn->setEpoll(pE);

                pConn->getEpoll()->netEpollAdd(pConn, EPOLL_READABLE);
            
                log(Info, "Connect to %s:%u success", ip, uport);
*/
                return pConn;
            }
    };

    class TcpServer   : public IDataHandlerAware
    {
        public:
            
            explicit TcpServer(uint32_t port);

            ~TcpServer()
            {
                
            }

            void onError(){}
            void onWrite(){}
            void onRead();
                
            void startServer();

            void newConnection(int sockfd, const InetAddress& peerAddr);

            void removeConnection(const TcpConnPtr& conn);
        
        private:

            boost::scoped_ptr<Acceptor> acceptor; // avoid revealing Acceptor

            std::tr1::unordered_map<string, TcpConnPtr> connections;

//            CEpoll* ep;
    
    };
}

#endif
