#pragma  once

#include <errno.h>
#include <sys/epoll.h>
#include <map>


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
		class CEpoll
		{
			public:
				CEpoll();
				~CEpoll();
				int NetEpollInit();	
				int NetEpollAdd(int fd, int mask);
				int NetEpollDel(int fd, int mask);
				void NetEpollRun();

			private:

				uint32_t m_epfd;
				
				//fd->mask
				std::map<uint32_t, uint32_t> m_fdmap;

				bool m_bStop;
		};
	}
}