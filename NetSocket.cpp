 #include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "NetSocket.h"

using namespace server::net;
/*

int CNetSocketHelp::netSetSockReuse(int so)
{
	int yes = 1;
	if(setsockopt(so, SOL_SOCKET, SO_REUSEADDR, (const char*)yes, sizeof(yes)) != 0)
	{
		//	log(Error, "set sock SO_REUSERADDR err:%s", strerror(errno));
		return NET_ERR;
	}
	return NET_OK;	
}

int CNetSocketHelp::netCreateSocket(int domain)
{
	int s ;
	if(s = socket(domain, SOCK_STREAM, 0) == -1)
	{
	//	log(Error, "create sock err:%s", strerror(errno))
		return NET_ERR;
	}
	if(netSetSockReuse(s) != NET_OK)
	{
	//	log(Error, "set sock reuse err:%s", strerror(errno));
		close(s);
		return NET_ERR;
	}
	return s;
}


int CNetSocketHelp::netListen(int so, void* addr, socklen_t len, int backlog = 20)
{
	if(bind(so, (struct sockaddr*)addr, len) == -1)
	{
	//	log(Error, "bind sock err:%s", strerror(errno));
		close(so);
		return NET_ERR;
	}
	if(listen(so, backlog) != 0)
	{
	//	log(Error, "listen sock err:%s", strerror(errno));
		close(so);
		return NET_ERR;
	}
	return NET_OK;
}

int CNetSocketHelp::netSetNonBlock(char* err, int so)
{
	int flag ;
	
	if(flag = fcntl(so, F_GETFL) == -1)
	{
		//log(Error, "fcntl(F_GETFL) error:%s", strerror(errno));
		return NET_ERR;
	}
	if(fcntl(so, F_SETFL, flag | O_NONBLOCK) == -1)
	{
	//	log(Error, "fcntl(F_SETFL) error:%s", strerror(errno));
		return NET_ERR;
	}

	return NET_OK;
}

CNetConn* CNetSocketHelp::InitClientSock(const char* ip, uint32_t uport, CEpoll* pE, ILinkHander* linkHander, IDataHander* dataHander)
{
	int m_so = ::socket(AF_INET, SOCK_STREAM, 0);
	if(m_so == -1)
	{
	//	log(Error, "Create server Error:%s", ip);
		return NULL;
	}
	
	struct sockaddr_in sa;
	u_long addr = INADDR_ANY;
	if(ip != "")
	{
		addr = ::inet_addr(ip);
		if(addr == INADDR_ANY)
		{
			close(m_so);
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
		return NULL;
	}

	CNetConn* pConn = new CNetConn(m_so, sa.sin_addr.s_addr, uport, linkHander, dataHander);

	pConn->SetEpoll(pE);

	pConn->GetEpoll()->NetEpollAdd(m_so, EPOLL_READABLE);
	
	return pConn;
	//log(Info, "Connect to %s:%u success", ip, uport);
	
}

*/
////////////////////////////////////////////////////////////////////////////////


void CNetServer::StartServer(const char* ip, uint32_t uport)
{
	m_so = CNetSocketHelp::netCreateSocket(AF_INET);
	if(m_so == NET_ERR)
	{
	//	log(Error, "Create server Error:%s", ip);
		return;
	}
	
	u_long addr = INADDR_ANY;
	if (ip != NULL)
	{
		addr = ::inet_addr(ip);
		if (addr == INADDR_NONE)
		{
			close(m_so);
	//		throw exception_errno( toString("[CSocketHelper::createServerSocket] inet_addr(ip:%s)", ip) );
		}
	}
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = addr;
	sa.sin_port = htons(uport);

	int ret = CNetSocketHelp::netListen(m_so, (struct sockaddr*)&sa, sizeof(sa));
	if(ret == NET_ERR)
	{
	//	log(Error, "listen so:%u err", m_so);
		return;
	}

	GetEpoll()->NetEpollAdd(m_so, EPOLL_READABLE);
}

void CNetServer::OnRead()
{
	char addr[256];
	socklen_t* len;
	int m_fd = ::accept(m_so, (struct sockaddr*)addr, len);
	if(m_fd == -1)
	{
	//	log(Error, "accept error:%u", m_so);
		return;
	}

	//struct sockaddr_in* in_addr = (struct sockaddr_in*)addr;
	//CNetConn* conn = new CNetConn(m_fd, in_addr->sin_addr.s_addr, ntohs(in_addr->sin_port), getLinkHander(), getDataHander());
	
	GetEpoll()->NetEpollAdd(m_fd, EPOLL_READABLE);

	//log(Info, "Accpet from:%s : %u", inet_ntoa(in_addr->sin_addr), in_addr->sin_port);
}