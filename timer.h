#ifndef TIMER_H_
#define TIMER_H_

#include <sys/time.h>
#include <list>
#include <boost/thread/mutex.hpp>
#include "event_source.h"

namespace znb
{
	class ITimer
	{
	public:
		virtual ~ITimer() {}
		virtual void OnTimer() = 0;
	};

	class CTimerMgr : public CEvSource
	{
	public:
		CTimerMgr();
		~CTimerMgr();

	public:
		virtual void onRead();
        virtual void onWrite(){}
        virtual void onError(){}

		void Init(CEpoll* selector);
		void AddTimeout(ITimer* p, int msecs);
		void RemoveTimeout(ITimer* p);

	private:
		void __ResetTimer();
		void __OnTimer();
		void __CheckTimeout();

	private:
		bool m_bInCallback;
		struct timeval m_tvNow;

		struct TimerStruct
		{
		public:
			TimerStruct(ITimer* p, const struct timeval& tv)
			: obj(p), tmo(tv)
			{
			}

		public:
			ITimer* obj;
			struct timeval tmo;
		};

		boost::mutex m_lstLock;
		std::list<TimerStruct> m_lstTimers;
	};
}

#endif // TIMER_H_
