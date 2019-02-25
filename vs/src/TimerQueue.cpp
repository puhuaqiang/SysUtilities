#include "../SysUtilities/stdafx.h"
#include "../include/TimerQueue.h"
#include "../include/AutoLock.h"

namespace SYS_UTL
{
	void CALLBACK TimerRoutine(void* cb, BOOLEAN TimerOrWaitFired)
	{
		CTimer* timer = reinterpret_cast<CTimer*>(cb);
		if (NULL != timer)
		{
			timer->run();
		}
	}

	CTimerQueue::CTimerQueue() :
		timerQueue_(CreateTimerQueue())
	{
		assert(NULL != timerQueue_);
		check_ = addTimer([this](){
			SYS_UTL::CAutoLock lck(&mutex_);
			auto itbg = timers_.begin();
			auto itend = timers_.end();
			for (; itbg != itend;)
			{
				if (itbg->first->is_die())
				{
					delete itbg->first;
					itbg = timers_.erase(itbg);
					continue;
				}
				itbg++;
			}
		}, CTimestamp(), 60);
	}

	CTimerQueue::~CTimerQueue()
	{
		DeleteTimerQueueEx(timerQueue_, INVALID_HANDLE_VALUE/*NULL*/);
		for (auto& it : timers_)
		{
			delete it.first;
		}
	}

	CTimerId CTimerQueue::addTimer(const TimerCallback& cb, CTimestamp when, double seconds)
	{
#if 1
		SYS_UTL::CAutoLock lck(&mutex_);
		CTimestamp now = CTimestamp::now();
		double timeDif = timeDifference(when, now);
		if (timeDif < 0)
		{
			if (seconds <= 0)
			{
				return CTimerId(NULL, 0);
			}
		}
		CTimer* timer = new CTimer(std::move(cb), when, seconds);
		HANDLE htimer;
		bool once = seconds > 0 ? false : true;
		BOOL success = CreateTimerQueueTimer(&htimer,
			timerQueue_,
			(WAITORTIMERCALLBACK)TimerRoutine, timer,
			once ? static_cast<DWORD>(timeDif * 1000) : 0,
			once ? 0 : static_cast<DWORD>(seconds * 1000), 0);
		assert(success);
		//intptr_t id = reinterpret_cast<intptr_t>(timer);
		timers_[timer] = htimer;
		return CTimerId(timer, timer->sequence());
#else
		std::shared_ptr<TimerCallback> task(new TimerCallback(cb));
		HANDLE timer;
		bool once = interval ? false : true;
		bool success = CreateTimerQueueTimer(&timer, timerQueue_, (WAITORTIMERCALLBACK)TimerRoutine, task.get(), 0, once ? 0 : interval, 0);
		if (success)
		{
			intptr_t id = reinterpret_cast<intptr_t>(timer);
			timers_[id] = task;
			return id;
		}
		return 0;
#endif
	}

	void CTimerQueue::cancel(CTimerId timerId)
	{
		SYS_UTL::CAutoLock lck(&mutex_);
		auto it = timers_.find(timerId.timer_);
		if (it != timers_.end())
		{
			DeleteTimerQueueTimer(timerQueue_, it->second, NULL);//Á¢¼´·µ»Ø
			//DeleteTimerQueueTimer(timerQueue_, it->second, INVALID_HANDLE_VALUE);
			//timers_.erase(it);
			it->first->die();
		}
	}

	std::atomic<int64_t> CTimer::s_numCreated_ = 0;

	void CTimer::restart(CTimestamp now)
	{
		if (repeat_)
		{
			expiration_ = addTime(now, interval_);
		}
		else
		{
			expiration_ = CTimestamp::invalid();
		}
	}

}
