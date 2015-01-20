 #include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "NetSocket.h"

using namespace server::net;

void CNetServer::StartServer(const char* ip, uint32_t uport)
{
	m_fd = CNetSocketHelp::netCreateSocket(AF_INET);
	if(m_fd == NET_ERR)
	{
		Log(Error, "Create server Error:%s", ip);
		return;
	}
	
	u_long addr = INADDR_ANY;
	if (ip != NULL)
	{
		addr = ::inet_addr(ip);
		if (addr == INADDR_NONE)
		{
			close(m_fd);
			Log(Error, "addr is NUll");
			return;
		}
	}
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = addr;
	sa.sin_port = htons(uport);

	int ret = CNetSocketHelp::netListen(m_fd, (struct sockaddr*)&sa, sizeof(sa));
	if(ret == NET_ERR)
	{
		Log(Error, "listen so:%u err", m_fd);
		return;
	}

	GetEpoll()->NetEpollAdd(this, EPOLL_READABLE);
}

void CNetServer::OnRead()
{
	struct sockaddr_in addr;
	socklen_t len = 1;
	memset(&addr, 0, sizeof(addr));
	int m_so = ::accept(m_fd, (struct sockaddr*)&addr, &len);
	if(m_so == -1)
	{
		Log(Error, "accept fd:so %u:%u,err:%s", m_fd,m_so, strerror(errno));
		return;
	}

	CNetConn* conn = new CNetConn(m_so, addr.sin_addr.s_addr, ntohs(addr.sin_port), getLinkHander(), getDataHander());
	
	conn->SetEpoll(GetEpoll());

	GetEpoll()->NetEpollAdd(conn, EPOLL_READABLE);

	Log(Info, "Accpet from:%s : %u", inet_ntoa(addr.sin_addr), addr.sin_port);
}