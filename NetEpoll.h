#pragma  once

#include <errno.h>
#include <sys/epoll.h>
#include <set>
#include <string.h>
#include "Logger.h"


namespace server{

	namespace net
	{
		enum EPOLL_STAT
		{
			EPOLL_NONE,
			EPOLL_READABLE,
			EPOLL_WRITEABLE,
			EPOLL_ERR
		};
		enum SOCKET_STAT
		{
			NET_OK,
			NET_ERR
		};
		
		class CEvSource;
		class CEpoll
		{
			public:
				CEpoll();
				~CEpoll();
				int NetEpollInit();	
				int NetEpollAdd(CEvSource* pEv, int mask);
				int NetEpollDel(CEvSource* pEv, int mask);
				void NetEpollRun();
				void remove(CEvSource* pEv);

			private:

				uint32_t m_epfd;
				
				//fd->mask
				std::set<CEvSource*> m_setEv;

				bool m_bStop;
		};
	}
}