#include "acceptor.h"

using namespace znb;

void Acceptor::handleRead()
{
    InetAddress peerAddr;
    char addr[256];
    socklen_t len = 256;
    int connfd = ::accept(socketfd_, (struct sockaddr*)&addr, &len);

    if (connfd >= 0)
    {
        if( fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFD, 0)|O_NONBLOCK) == -1  )
        {
            log(Error, "[Acceptor::handleRead] set [ %u  ] nonblock err:%s", connfd, strerror(errno));
            return;
        }
        peerAddr.setSockAddrInet(*reinterpret_cast<struct sockaddr_in*>(addr));
        // string hostport = peerAddr.toIpPort();
        // LOG_TRACE << "Accepts of " << hostport;
        if (newConnectionCallback)
        {
            newConnectionCallback(connfd, peerAddr);
        }
        else
        {
            ::close(connfd);
        }
    }
    else
    {
        log(Error, "[handleRead] in Acceptor::handleRead");
        // Read the section named "The special problem of
        // accept()ing when you can't" in libev's doc.
        // By Marc Lehmann, author of livev.
    
    }
}
