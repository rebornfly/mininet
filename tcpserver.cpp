#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "tcpserver.h"

using namespace znb;

TcpServer::TcpServer(uint32_t port)
    :acceptor(new Acceptor(CNetSocketHelp::initListenSockfd(port)))
{
    acceptor->setnewConnectionCallback(boost::bind(&TcpServer::newConnection, this, _1, _2));
}

void TcpServer::startServer()
{
        acceptor->listen();
        log(Info, "TcpServer::startServer...... " );
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
    string name = peerAddr.toIpPort();
    TcpConnPtr conn(new CTcpConn(Globals::GetEpoll(), sockfd, inet_ntoa(peerAddr.addr_.sin_addr), ntohs(peerAddr.addr_.sin_port),  getDataHandler()));
    
    connections[name] = conn;

    conn->setConnStat(ENUM_STATE_CONNECTED);
    conn->setCloseCallback(
      boost::bind(&TcpServer::removeConnection, this, _1));

    log(Info, "[onAccept] Accept from:%s : %u sockid:%u ", inet_ntoa(peerAddr.addr_.sin_addr), peerAddr.addr_.sin_port, sockfd);
}

void TcpServer::removeConnection(const TcpConnPtr& conn)
{
   InetAddress addr( conn->getPeerPort(), conn->getPeerIp());
   string name = addr.toIpPort();
   connections.erase(name);
   log(Info,"[TcpServer::removeConnection] remove conn :%s", name.c_str());
}
