#include "tcpservermt.h"

using namespace znb;

TcpServerMt::TcpServerMt(uint32_t port)
: TcpServer(port)
{
    // 创建工作者线程
    for (int i = 0; i < WorkerSize; i++)
    {
        m_workers.push_back(new Worker(this));
    }
}

TcpServerMt::~TcpServerMt()
{
}

void TcpServerMt::newConnection(int sockfd, const InetAddress& peerAddr)
{

    // 将连接分配到某一个线程
    int nMinLoad = 65535;
    Worker* pMinLoad = 0;
    for (Workers::iterator it = m_workers.begin(); it != m_workers.end(); ++it)
    {
        int nLoad = (*it)->getLoad();
        if (nLoad < nMinLoad)
        {
            nMinLoad = nLoad;
            pMinLoad = *it;
        }
    }

    pMinLoad->postConn(sockfd);

}

// 主线程调用
TcpServerMt::Worker::Worker(IDataHandlerAware* aware)
:m_nNumConn(0)
,dataHandler(aware)
{
    if (pipe(m_pipefd) == -1)
    {
        throw Socket_Exception("pipe");
    }

    // 监控管道的读端
    pipecaller.reset(new PipeCaller(m_pipefd[0], &m_pE));
    
    pipecaller->setnewConnectionCallback(boost::bind(&TcpServerMt::Worker::handleRead, this));

    start();
}

TcpServerMt::Worker::~Worker()
{
}

// 工作者线程调用
void TcpServerMt::Worker::process()
{
    pipecaller->listen();

    m_pE.netEpollRun();
}

int TcpServerMt::Worker::getLoad()
{
    return m_nNumConn;
}

// 主线程调用
void TcpServerMt::Worker::postConn(uint32_t uConnId)
{
    ++m_nNumConn;

    // 将连接id传递到工作者线程
    if (write(m_pipefd[1], &uConnId, sizeof(uint32_t)) != sizeof(uint32_t))
    {
        log(Error, "Send notify event to worker failed with error: %s", strerror(errno));
    }
}

// 工作者线程调用
void TcpServerMt::Worker::handleRead()
{
    uint32_t sockfd = 0;
    int ret = 0;
    ret = ::read(m_pipefd[0], &sockfd, sizeof(uint32_t));
    if (ret != sizeof(uint32_t))
    {
        log(Error, "read returns %d", ret);
        return;
    }

    struct sockaddr_in sa;
    socklen_t  len = sizeof(sa);
    if(getpeername(sockfd, (struct sockaddr *)&sa, &len))
    {
        throw Socket_Exception("getpeername");
    }
    TcpConnPtr conn(new CTcpConn(&m_pE, sockfd, inet_ntoa(sa.sin_addr),  ntohs(sa.sin_port),  dataHandler->getDataHandler()));
    
    conns[sockfd] = conn;

    conn->setConnStat(ENUM_STATE_CONNECTED);
    conn->setCloseCallback(
    boost::bind(&TcpServerMt::Worker::removeConnection, this, _1));

    conn->setBlocking(false);
    log(Info, "tid:%u Number of Connections:%d", __getThreadId(), m_nNumConn);

}

void TcpServerMt::Worker::removeConnection(const TcpConnPtr& conn)
{
   conns.erase(conn->getFd());
   log(Info,"[TcpServer::removeConnection] remove conn :%s|%u %u", conn->getPeerIp().c_str(), conn->getPeerPort(), conn->getFd());
}
