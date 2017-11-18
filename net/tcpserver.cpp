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
    
    connections[conn->getFd()] = conn;

    conn->setConnStat(ENUM_STATE_CONNECTED);
    conn->setBlocking(false);
    conn->setCloseCallback(
      boost::bind(&TcpServer::removeConnection, this, _1));

    log(Info, "[onAccept] Accept from:%s : %u sockid:%u ", inet_ntoa(peerAddr.addr_.sin_addr), peerAddr.addr_.sin_port, sockfd);
}

TcpConnPtr TcpServer::clientConnection(const char* ip, uint32_t port)
{
    TcpConnPtr conn = CNetSocketHelp::clientConnection(ip, port, Globals::GetEpoll(), getDataHandler());

    connections[conn->getFd()] = conn;

    conn->setConnStat(ENUM_STATE_CONNECTED);
    conn->setBlocking(false);
    conn->setCloseCallback(
      boost::bind(&TcpServer::removeConnection, this, _1));

    log(Info, "[onAccept] accept to:%s : %u sockid:%u ", ip,port, conn->getFd());
    return conn;
}

void TcpServer::removeConnection(const TcpConnPtr& conn)
{
   connections.erase(conn->getFd());
   log(Info,"[TcpServer::removeConnection] remove conn %s:%u", conn->getPeerIp().c_str(), conn->getPeerPort());
}
