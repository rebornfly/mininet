#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>

#include <sys/timerfd.h>
#include <strings.h>
#include <cassert>
#include "timer.h"
#include "logger.h"

using namespace znb;

CTimerMgr::CTimerMgr()
: m_bInCallback(false),
m_timerfd(timerfd_create(CLOCK_MONOTONIC, 0)),
ev(new CEvSource(m_timerfd))
{
    ev->setReadCallback(boost::bind(&CTimerMgr::onRead, this));
}

CTimerMgr::~CTimerMgr()
{
}

void CTimerMgr::onRead()
{
    __OnTimer();
}

void CTimerMgr::Init(CEpoll* ep)
{
/*    m_fd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (m_fd == -1)
    {
        throw ZNB_Exception("timerfd_create");
    }*/
    // �������߳�
//    Select(sox::SEL_READ, 0);
    ep->netEpollAdd(ev.get(), EPOLL_READABLE);
    log(Info, "[TEST::CTimerMgr::Init] fd:%u", m_timerfd);
}

void CTimerMgr::AddTimeout(ITimer* p, int msecs)
{
    if (!m_bInCallback)
    {
        gettimeofday(&m_tvNow, NULL);
    }

    // �õ�ǰʱ����϶�ʱֵ, ���Ƕ�ʱ���Ĵ���ʱ��
    struct timeval tv = m_tvNow;
    if (msecs >= 1000)
    {
        // С�����
        tv.tv_sec += msecs / 1000;
        msecs %= 1000;
    }

    tv.tv_usec += msecs * 1000;
    if (tv.tv_usec >= 1000000)
    {
        tv.tv_sec += tv.tv_usec / 1000000;
        tv.tv_usec %= 1000000;
    }

    bool bNeedReset = false;
    {
        boost::mutex::scoped_lock sl(m_lstLock);

        // ������ʱ��˳�����
        std::list<TimerStruct>::iterator it = m_lstTimers.begin();
        for (; it != m_lstTimers.end(); ++it)
        {
            if (it->tmo.tv_sec > tv.tv_sec || (it->tmo.tv_sec == tv.tv_sec && it->tmo.tv_usec > tv.tv_usec))
            {
                break;
            }
        }

        if (it == m_lstTimers.begin())
        {
            // �ö�ʱ�����뵽������ǰ��
            if (!m_bInCallback)
            {
                bNeedReset = true;
            }
        }
        m_lstTimers.insert(it, TimerStruct(p, tv));
    }

    // ��CTimerMgr���ڻص��������Զ����趨ʱ��, ���Խ���AddTimeout���ڳ�ʱ�ص��������������趨ʱ��
    // �ҽ����ó�ʱ��Ϊ��һ���������ڵĶ�ʱ��ʱ����
    if (bNeedReset)
    {
        __ResetTimer();
    }
}

void CTimerMgr::RemoveTimeout(ITimer* p)
{
    boost::mutex::scoped_lock sl(m_lstLock);

    // ������ʱ��˳�����
    for (std::list<TimerStruct>::iterator it = m_lstTimers.begin(); it != m_lstTimers.end();)
    {
        if (it->obj == p)
        {
            it = m_lstTimers.erase(it);
        }
        else
        {
            it++;
        }
    }
}

void CTimerMgr::__ResetTimer()
{
    struct timeval tvNext;
    {
        boost::mutex::scoped_lock sl(m_lstLock);
        if (m_lstTimers.empty())
            return;

        tvNext = m_lstTimers.front().tmo;
    }

    struct itimerspec newVal;
    bzero(&newVal, sizeof(struct itimerspec));

    time_t second = tvNext.tv_sec - m_tvNow.tv_sec;
    long usecond = tvNext.tv_usec - m_tvNow.tv_usec;
    if (usecond < 0)
    {
        second--;
        usecond += 1000000;
    }

    newVal.it_value.tv_sec = second;
    newVal.it_value.tv_nsec = usecond * 1000;

    if (timerfd_settime(m_timerfd, 0, &newVal, NULL) == -1)
    {
        log(Error, "timerfd_settime failed with error:%s", strerror(errno));
        log(Error, "newVal tv_sec:%" PRIu64 ", tv_nsec:%" PRIu64 "", uint64_t(newVal.it_value.tv_sec), uint64_t(newVal.it_value.tv_nsec));
        assert(false);
    }
}

void CTimerMgr::__OnTimer()
{
    uint64_t exp;
    ssize_t s = read(m_timerfd, &exp, sizeof(uint64_t));
    if (s != sizeof(uint64_t))
    {
//        throw exception_errno("read");
    }

    // ��鵽�ڵĶ����������ص�
    gettimeofday(&m_tvNow, NULL);
    __CheckTimeout();
    __ResetTimer();
}

void CTimerMgr::__CheckTimeout()
{
    std::list<ITimer *> lstTmoObjs;

    {
        boost::mutex::scoped_lock sl(m_lstLock);
        if (m_lstTimers.empty())
            return;

        for (std::list<TimerStruct>::iterator it = m_lstTimers.begin(); it != m_lstTimers.end();)
        {
            if (it->tmo.tv_sec < m_tvNow.tv_sec || (it->tmo.tv_sec == m_tvNow.tv_sec && it->tmo.tv_usec <= m_tvNow.tv_usec))
            {
                lstTmoObjs.push_back(it->obj);

                // �Ӷ�����ɾ��
                it = m_lstTimers.erase(it);
            }
            else
            {
                break;
            }
        }
    }

    if (!lstTmoObjs.empty())
    {
        for (std::list<ITimer *>::iterator it = lstTmoObjs.begin(); it != lstTmoObjs.end(); ++it)
        {
            m_bInCallback = true;
            (*it)->OnTimer();
            m_bInCallback = false;
        }
    }
}
