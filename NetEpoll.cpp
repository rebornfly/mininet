#include "NetEpoll.h"
#include "EvSource.h"

using namespace server::net;

CEpoll::CEpoll():
m_bStop(false)
{
	
}
CEpoll::~CEpoll()
{

}

int CEpoll::NetEpollInit()
{
	
	if((m_epfd = epoll_create(1024)) == -1)
	{
		Log(Error, "epoll create :%s", strerror(errno));
		return NET_ERR;
	}
	
	return NET_OK;
}

int CEpoll::NetEpollAdd(CEvSource* pEv, int mask)
{
	int op;
	if(m_setEv.find(pEv) == m_setEv.end())
	{
		op = EPOLL_CTL_ADD;
	}
	else 
	{
		op = EPOLL_CTL_MOD;
		mask |= pEv->getCurrentMask();
	}
	
	struct epoll_event es;
	es.events = 0;
	es.data.ptr = pEv;
	if(mask & EPOLL_READABLE) es.events |= EPOLLIN;
	if(mask & EPOLL_WRITEABLE) es.events |= EPOLLOUT;

	if(epoll_ctl(m_epfd, op, pEv->getFd(), &es) == -1)
	{
		Log(Error, "epoll_ctrl err:%s", strerror(errno));
		return NET_ERR;
	}
	
	//¸úÐÂÊÂ¼þ 
	m_setEv.insert(pEv);
	pEv->setCurrentMask(mask);
	return NET_OK;
}

int CEpoll::NetEpollDel(CEvSource* pEv , int mask)
{
	int op ;
	struct epoll_event ee;
	ee.data.ptr = pEv;
	ee.events = 0;
	ee.data.u64 = 0;

	int delmask = ~mask & pEv->getCurrentMask();
	if(delmask == EPOLL_NONE)
	{
		op = EPOLL_CTL_DEL;
	}
	else
	{
		op = EPOLL_CTL_MOD;
	}

	ee.events =  delmask;

	if(epoll_ctl(m_epfd, op, pEv->getFd(), &ee) == -1)
	{
		Log(Error, "epoll_ctrl del err:%s", strerror(errno));
		return NET_ERR;
	}

	if(delmask == EPOLL_NONE)
	{
		m_setEv.erase(pEv);
		pEv->setCurrentMask(EPOLL_NONE);
	}
	else 
	{
		pEv->setCurrentMask(delmask);
	}

	return NET_OK;

}

void CEpoll::NetEpollRun()
{
	epoll_event events[1024];
	while (!m_bStop)
	{
		int waits = epoll_wait(m_epfd, events, m_setEv.size(), -1);
		for(int i = 0; i < waits; i++)
		{
			int mask = EPOLL_NONE;
			CEvSource* ev = events[i].data.ptr;
			if(events[i].events & EPOLLIN)
			{
				ev->OnRead();
			}
			if(events[i].events & EPOLLOUT)
			{
				ev->OnWrite();
			}
			if(events[i].events & EPOLLERR || events[i].events & EPOLLHUP)
			{
				ev->OnError();
			}
			
		}
	}
}

void CEpoll::remove(CEvSource* pEv)
{
	if(m_setEv.find(pEv) != m_setEv.end())
	{
		struct epoll_event ev;
		epoll_ctl(m_epfd, EPOLL_CTL_DEL , pEv->getFd(), &ev);
		m_setEv.erase(pEv);
		Log(Info, "erase fd:%u from epoll", pEv->getFd());
	}
}