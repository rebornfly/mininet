#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include <boost/thread/thread.hpp>
#include <tr1/unordered_map>
#include "event_source.h"
#include "net_conn.h"
#include "acceptor.h"
#include "socket_help.h"
namespace znb
{
    
    class TcpServer : public IDataHandlerAware
    {
        public:
            explicit TcpServer(uint32_t port);

            ~TcpServer()
            {
            }
            void startServer();

            virtual void newConnection(int sockfd, const InetAddress& peerAddr);

            void removeConnection(const TcpConnPtr& conn);

            TcpConnPtr clientConnection(const char* ip, uint32_t port);

        protected:

            std::tr1::unordered_map<uint32_t, TcpConnPtr> connections;

            std::tr1::unordered_map<uint32_t, TcpConnPtr> clientconns;

        private:

            boost::scoped_ptr<Acceptor> acceptor; // avoid revealing Acceptor

    };
}

#endif
