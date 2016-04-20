#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "tcpserver.h"

using namespace znb;

void TcpServer::startServer(uint32_t uport)
{
	try
	{
		m_fd = CNetSocketHelp::initListenSockfd(uport);
		getEpoll()->netEpollAdd(this, EPOLL_READABLE);
	}
	catch(Socket_Exception& e)
	{
		log(Info, "TcpServer::startServer error:%s", e.what());
	}
}

void TcpServer::onRead()
{
	//监听套接字事件
	if(getLinkHandler())
	{
		getLinkHandler()->onAccept(getEpoll(), m_fd, getDataHandler());
	}
}

