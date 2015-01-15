#include "NetEpoll.h"

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
	//	setErr(err, "epoll create :%s", strerror(errno));
		return NET_ERR;
	}
	
	return NET_OK;
}

int CEpoll::NetEpollAdd( int fd, int mask)
{
	int op;
	if(m_fdmap.find(fd) == m_fdmap.end()) op = EPOLL_CTL_ADD;
	else 
	{
		op = EPOLL_CTL_MOD;
		mask |= m_fdmap[fd];
	}
	
	struct epoll_event es;
	es.events = 0;
	es.data.fd = fd;
	if(mask & EPOLL_READABLE) es.events |= EPOLLIN;
	if(mask & EPOLL_WRITEABLE) es.events |= EPOLLOUT;

	if(epoll_ctl(m_epfd, op, fd, &es) == -1)
	{
	//	setErr("epoll_ctrl err:%s", strerror(errno));
		return NET_ERR;
	}
	
	//¸úÐÂÊÂ¼þ 
	m_fdmap[fd] = mask;
	return NET_OK;
}

int CEpoll::NetEpollDel( int fd , int mask)
{
	int op ;
	struct epoll_event ee;
	ee.data.fd = fd;
	ee.events = 0;
	ee.data.u64 = 0;

	int delmask = ~mask & m_fdmap[fd];
	if(delmask == EPOLL_NONE) op = EPOLL_CTL_DEL;
	else op = EPOLL_CTL_MOD;

	ee.events =  delmask;

	if(epoll_ctl(m_epfd, op, fd, &ee) == -1)
	{
	//	setErr(err, "epoll_ctrl del err:%s", strerror(errno));
		return NET_ERR;
	}

	if(delmask == EPOLL_NONE) m_fdmap.erase(fd);
	else m_fdmap[fd] = delmask;

	return NET_OK;

}

void CEpoll::NetEpollRun()
{
	epoll_event events[1024];
	while (!m_bStop)
	{
		int waits = epoll_wait(m_epfd, events, m_fdmap.size(), -1);
		for(int i = 0; i < waits; i++)
		{
			int mask = EPOLL_NONE;
			if(events[i].events & EPOLLIN)
				mask |= EPOLL_READABLE;
			if(events[i].events & EPOLLOUT)
				mask |= EPOLL_WRITEABLE;
			if(events[i].events & EPOLLERR || events[i].events & EPOLLHUP)
				mask |= EPOLL_ERR;
		}
	}
}