#include "net_epoll.h"
#include "event_source.h"

using namespace znb;

CEpoll::CEpoll():
m_eventFd(createEventFd()),
wakeupEv(new WakeupEvent(m_eventFd)),
m_bcallingPendingFunctors(false),
m_bStop(false)
{
	m_epfd = epoll_create(1024);
	if(m_epfd == -1)
	{
		log(Error, "epoll create :%s", strerror(errno));	
        abort();
	}

    netEpollAdd(wakeupEv, EPOLL_READABLE);
    log(Info, "[TEST::CEpoll]::create eventfd:%u", m_eventFd);
}
CEpoll::~CEpoll()
{	
    ::close(m_epfd);
    ::close(m_eventFd);
}

int CEpoll::netEpollAdd(CEvSource* pEv, int mask)
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

    //�����¼� //
	if(op == EPOLL_CTL_ADD) m_setEv.insert(pEv);
	pEv->setCurrentMask(mask);

    log(Warn, "[TEST::netEpollAdd] es:%p, op:%u, mask:%u, fd:%d", pEv, op, mask, pEv->getFd());

	if(epoll_ctl(m_epfd, op, pEv->getFd(), &es) == -1)
	{
		log(Error, "[netEpollAdd]epoll_ctrl es:%p, op:%u err:%s", pEv, op, strerror(errno));
		return NET_ERR;
	}
	
	return NET_OK;
}

int CEpoll::netEpollDel(CEvSource* pEv , int mask)
{
	int op ;
	struct epoll_event ee;
	ee.data.ptr = pEv;
	ee.events = 0;

	int delmask = ~mask & pEv->getCurrentMask();

	if(delmask == EPOLL_NONE)
	{
		op = EPOLL_CTL_DEL;
	}
	else
	{
		op = EPOLL_CTL_MOD;
	}

	if(delmask & EPOLL_READABLE) ee.events |= EPOLLIN;
	if(delmask & EPOLL_WRITEABLE) ee.events |= EPOLLOUT;

	if(epoll_ctl(m_epfd, op, pEv->getFd(), &ee) == -1)
	{
		log(Error, "epoll_ctrl del err:%s", strerror(errno));
		return NET_ERR;
	}

 //   log(Warn, "[netEpollDel] fd:%u es:%p, delmask:%u", pEv->getFd(), pEv, delmask);

	if(delmask == EPOLL_NONE)
	{
		m_setEv.erase(pEv);
		m_rmSet.insert(pEv);
	}
	else 
	{
		pEv->setCurrentMask(delmask);
	}

	return NET_OK;

}

void CEpoll::netEpollRun()
{
	epoll_event events[1024];
	while (!m_bStop)
	{
		int waits = epoll_wait(m_epfd, events, m_setEv.size(), -1);
		if(-1 == waits && errno == EINTR)
		{
			log(Warn, "epoll_wait err:%s", strerror(errno));
			continue;
		}
		
		for(int i = 0; i < waits; i++)
		{
			CEvSource* ev = (CEvSource* )events[i].data.ptr;
			if(events[i].events & EPOLLIN)
			{
				if (ev->readCallback_) ev->readCallback_();
			}
			if(events[i].events & EPOLLOUT)
			{
				if (ev->writeCallback_) ev->writeCallback_();
			}
			if(events[i].events & EPOLLERR || events[i].events & EPOLLHUP)
			{
				if (ev->errorCallback_) ev->errorCallback_();
			}
			
		}
		
		// 
		if (!m_rmSet.empty())		
		{
			for (std::set<CEvSource*>::iterator it = m_rmSet.begin(); it != m_rmSet.end(); )
			{			
				CEvSource* es = *it;	
			//	if(es->getEventType() == ENUM_TYPE_CONN)
			//	{
                    log(Warn, "[netEpollRunNEW] erase fd:%u--%p from epoll", es->getFd(), es);
				//	delete es;
					m_rmSet.erase(it++);
			//	}
			}
        
		}
        doingPendingFunctors();
	}
}

void CEpoll::remove(CEvSource* pEv)
{
	if(m_setEv.find(pEv) != m_setEv.end())
	{
		struct epoll_event ev;

		epoll_ctl(m_epfd, EPOLL_CTL_DEL , pEv->getFd(), &ev);

		m_setEv.erase(pEv);

		m_rmSet.insert(pEv);

        log(Warn, "[Remove]  fd:%d", pEv->getFd());
	}
}

void CEpoll::netEpollStop()
{
    m_bStop = false;
}

int CEpoll::createEventFd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);

    if (evtfd < 0)
    {
        log(Error, " create event fd failed %s !!!!", strerror(errno));
        abort();
    }

    return evtfd;
}

void CEpoll::wakeup()
{
    uint64_t u = 1;

    ssize_t flag = ::write(m_eventFd, &u, sizeof(uint64_t));
    log(Info, "[TEST::wakeup] m_eventFd:%u", m_eventFd);
    if(flag != sizeof(uint64_t))
    {
        log(Error, "[CEpoll::wakeup] flag:%lu, err:%s", flag ,strerror(errno));
    }
}

void CEpoll::doingPendingFunctors()
{
    m_bcallingPendingFunctors = true;
    vector<Functor> functors;

    {
        MutexGuard lock(m_mutex);
        functors.swap(m_vecPendingFunctors);
    }

    for(size_t i = 0; i < functors.size(); i++)
    {
        functors[i]();
    }
    m_bcallingPendingFunctors = false;
    //log(Info, "[doingPendingFunctors] tid:%u", (unsigned int)pthread_self());
}

void CEpoll::pushFuctor( Functor& fun)
{
    {
        MutexGuard lock(m_mutex);
        m_vecPendingFunctors.push_back(fun);
    }

    if (!m_bcallingPendingFunctors)
    {
        wakeup();
    }
    log(Info, "[pushFunctors] tid:%u", (unsigned int)pthread_self());
}
