#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "net_epoll.h"
#include "timer.h"

namespace znb
{
    class CEnvTimer;

    class Globals
    {
    public:
        static inline CEpoll* GetEpoll()
        {
            return m_pSelector;
        }

        static void SetSelector(CEpoll* pSelector);

        static inline CTimerMgr* GetTimerMgr()
        {
            return m_pTimerMgr;
        }

        static void SetTimerMgr(CTimerMgr* pTimerMgr);

        static inline CEnvTimer* GetEnvTimer()
        {
            return m_pEvnTimer;
        }

        static void SetEnvTimer(CEnvTimer* pEnvTimer);

    private:
        static CEpoll* m_pSelector;
        static CTimerMgr* m_pTimerMgr;

        static CEnvTimer* m_pEvnTimer;
    };

    class CEnvTimer
        : public ITimer
    {
    public:
        friend class Globals;

        CEnvTimer();
        virtual ~CEnvTimer();
        
        virtual void OnTimer();
        time_t inline getTime()
        {
            return m_tmNow;
        }

    protected:
        time_t m_tmNow;
    };
}

#endif // GLOBALS_H_
