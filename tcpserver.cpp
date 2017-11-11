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
//        m_fd = CNetSocketHelp::initListenSockfd(uport);
        acceptor->listen();
        log(Info, "TcpServer::startServer...... " );
}

/*()void TcpServer::onRead()
{
    //¼àÌýÌ×½Ó×ÖÊÂ¼þ
    if(getLinkHandler())
    {
        getLinkHandler()->onAccept(getEpoll(), m_fd, getDataHandler());
    }
}
*/
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
  /*loop_->assertInLoopThread();
  EventLoop* ioLoop = threadPool_->getNextLoop();
  char buf[32];
  snprintf(buf, sizeof buf, ":%s#%d", hostport_.c_str(), nextConnId_);
  ++nextConnId_;
  string connName = name_ + buf;

  log(Info, "TcpServer::newConnection [ %s] - new connection [ %s] from %u" , buf, connName.c_str(), sockfd);
  InetAddress localAddr(sockets::getLocalAddr(sockfd));
  // FIXME poll with zero timeout to double confirm the new connection
  // FIXME use make_shared if necessary
  TcpConnectionPtr conn(new TcpConnection(ioLoop,
                                          connName,
                                          sockfd,
                                          localAddr,
                                          peerAddr));
  connections_[connName] = conn;
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(
      boost::bind(&TcpServer::removeConnection, this, _1)); // FIXME: unsafe
  ioLoop->runInLoop(boost::bind(&TcpConnection::connectEstablished, conn)); */
/*char addr[256];
    socklen_t len = 256;
    int m_so = ::accept(sockfd, (struct sockaddr*)&addr, &len);
    if(m_so == -1)
    {
        log(Error, " accept fd:so [%u] err:%s",  sockfd, strerror(errno));
        return;
    }

    if( fcntl(m_so, F_SETFL, fcntl(m_so, F_GETFD, 0)|O_NONBLOCK) == -1 )
    {
        log(Error, "[onAccept]  set [ %u ] nonblock err:%s", m_so, strerror(errno));
        return;
    }
    
    struct sockaddr_in* addrsock = (struct sockaddr_in*)addr;
*/
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
