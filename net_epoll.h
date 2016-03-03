#ifndef _NET_EPOLL_H_
#define _NET_EPOLL_H_

#include <errno.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <set>
#include <string.h>
#include <vector>
#include <boost/function.hpp>
#include "logger.h"
#include "../common/znb_thread.h"



namespace znb
{
	/** @defgroup epoll模块儿
		* @author reborn-lys
		* @version 0.1
		* @date 2015.4.2
		* @{
	*/
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
    class WakeupEvent;
	class CEpoll
	{
		public:
            typedef boost::function<void()> Functor;

			CEpoll();
			~CEpoll();

			int  netEpollAdd(CEvSource* pEv, int mask);

			int  netEpollDel(CEvSource* pEv, int mask);

			void netEpollRun();

            void netEpollStop();

			void remove(CEvSource* pEv);
			
			uint32_t getSize()
			{
				return m_rmSet.size();
			}

            void pushFuctor(Functor& fun);

            /*唤醒I/O线程执行队列的functors*/
            void wakeup();

        private:

            /*执行队列里的functors*/
            void doingPendingFunctors();
            
            /*创造eventfd*/
            int createEventFd();

		private:

			int m_epfd;
				
			//fd->mask
			std::set<CEvSource*> m_setEv;
			
			/**待删除的事件集合*/
			std::set<CEvSource*> m_rmSet;

            /* 有子线程唤醒主线程要执行的函数 **/
            std::vector<Functor> m_vecPendingFunctors;

            int m_eventFd;

            WakeupEvent* wakeupEv;

            bool m_bcallingPendingFunctors;
    
			bool m_bStop;

            Mutex m_mutex;
	};
}
#endif
