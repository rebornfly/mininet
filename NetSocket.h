#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "EvSource.h"
#include "NetConn.h"

namespace server
{
	namespace net{

		class CNetSocketHelp{
			public:
				static int netSetSockReuse(int so)
				{
					int yes = 1;
					if(setsockopt(so, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) != 0)
					{
						Log(Error, "set sock SO_REUSERADDR err:%s", strerror(errno));
						return NET_ERR;
					}
					return NET_OK;	
				}
				static int netCreateSocket(int domain)
				{
					int s = socket(domain, SOCK_STREAM, 0);
					if(s == -1)
					{
						Log(Error, "create sock err:%s", strerror(errno));
						return NET_ERR;
					}
					if(netSetSockReuse(s) != NET_OK)
					{
						Log(Error, "set sock reuse err %d:%s", s, strerror(errno));
						close(s);
						return NET_ERR;
					}
					return s;
				}

				static int netListen(int so, void* addr, socklen_t len, int backLog = 20)
				{
					if(bind(so, (struct sockaddr*)addr, len) == -1)
					{
						Log(Error, "bind sock err:%s", strerror(errno));
						close(so);
						return NET_ERR;
					}
					if(listen(so, backLog) != 0)
					{
						Log(Error, "listen sock err:%s", strerror(errno));
						close(so);
						return NET_ERR;
					}
					return NET_OK;
				}


				static int netSetNonBlock(int so)
				{
					int flag  = fcntl(so, F_GETFL);

					if(flag == -1)
					{
						Log(Error, "fcntl(F_GETFL) error:%s", strerror(errno));
						return NET_ERR;
					}
					if(fcntl(so, F_SETFL, flag | O_NONBLOCK) == -1)
					{
						Log(Error, "fcntl(F_SETFL) error:%s", strerror(errno));
						return NET_ERR;
					}

					return NET_OK;
				}

				static CNetConn* InitClientSock(const char* ip, uint32_t uport, CEpoll* pE, ILinkHander* linkHander, IDataHander* dataHander)
				{
					int m_so = ::socket(AF_INET, SOCK_STREAM, 0);
					if(m_so == -1)
					{
						Log(Error, "Create server Error:%s", ip);
						return NULL;
					}

					struct sockaddr_in sa;
					u_long addr = INADDR_ANY;
					if(ip != NULL)
					{
						addr = ::inet_addr(ip);
						if(addr == INADDR_ANY)
						{
							close(m_so);
							Log(Error, "addr is NULL,");
							return NULL;
						}
					}
					
					sa.sin_family = AF_INET;
					sa.sin_port = htons(uport);
					sa.sin_addr.s_addr = addr;
					
					if(NET_OK != netSetNonBlock(m_so))
					{
						close(m_so);
						Log(Error, "Set NonBlock err:%s", strerror(errno));
						return NULL;	
					}
					int rc = ::connect(m_so, (struct sockaddr*)&sa, sizeof(sa));
					if(rc < 0)
					{
						close(m_so);
						Log(Error, "Client connet err:%s", strerror(errno));
						return NULL;
					}

					CNetConn* pConn = new CNetConn(m_so, sa.sin_addr.s_addr, uport, linkHander, dataHander);

					pConn->SetEpoll(pE);

					pConn->GetEpoll()->NetEpollAdd(pConn, EPOLL_READABLE);

					return pConn;
					Log(Info, "Connect to %s:%u success", ip, uport);

				}
		};

		class CNetServer :	public CEvSource
						   ,public IDataHanderAware
						   ,public ILinkHanderAware
		{
			public:

				CNetServer()
				{
					
				}
				~CNetServer()
				{
				
				}

				virtual void OnError(){}
				virtual void OnWrite(){}
				virtual void OnRead();
				
				void StartServer(const char* ip, uint32_t port);
				
		};
	}
}
