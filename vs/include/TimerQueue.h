#ifndef __SYS_UTILITIES_TIMER_QUEUE_H__
#define __SYS_UTILITIES_TIMER_QUEUE_H__

#include "SysUtilities.h"
#include "NonCopyAble.h"
#include "CritSec.h"
#include "Timestamp.h"
#include "CopyAble.h"

namespace SYS_UTL
{
	class CTimer;
	class CTimerId;

	typedef std::function<void()> TimerCallback;

	class SYS_UTL_CPPAPI CTimerId : public CCopyAble
	{
	public:
		CTimerId()
			: timer_(NULL),
			sequence_(0)
		{
		}

		CTimerId(CTimer* timer, int64_t seq)
			: timer_(timer),
			sequence_(seq)
		{
		}

		bool IsValid() const { return timer_ != NULL; }

		friend class CTimerQueue;

	private:
		CTimer* timer_;
		int64_t sequence_;
	};

	class SYS_UTL_CPPAPI CTimerQueue : CNonCopyAble
	{
	private:
		//typedef std::shared_ptr<std::function<void()>> Functor;
		HANDLE timerQueue_;
		std::map<CTimer*, HANDLE> timers_;
		CTimerId check_;
	public:
		CTimerQueue();
		~CTimerQueue();
		/**
		* @param when 定时器触发时间
		* @param interval 间隔时间 >0 周期性计时器. 单位毫秒
		*/
		CTimerId addTimer(const TimerCallback& cb, CTimestamp when, double seconds);
		void cancel(CTimerId timerId);
		SYS_UTL::CCritSec mutex_;
	};

	/**
	封装了定时器相关的事件和回调函数
	Timer封装了定时器的一些参数，例如超时回调函数、超时时间、定时器是否重复、重复间隔时间、定时器的序列号。
	其函数大都是设置这些参数，run()用来调用回调函数，restart()用来重启定时器（如果设置为重复）
	*/
	class SYS_UTL_CPPAPI CTimer : CNonCopyAble
	{
	public:
		CTimer(TimerCallback cb, CTimestamp when, double seconds)
			: callback_(std::move(cb)),//回调函数
			expiration_(when),//超时时间
			interval_(seconds),//如果重复，间隔时间
			repeat_(seconds > 0.0),//是否重复
			die_(false),
			sequence_(s_numCreated_++)//当前定时器的序列号
		{ }

		//超时时调用回调函数
		void run() const
		{
			assert(!is_die());
			callback_();
		}

		CTimestamp expiration() const  { return expiration_; }
		bool repeat() const { return repeat_; }
		int64_t sequence() const { return sequence_; }

		void restart(CTimestamp now);

		void die() { die_ = true; }
		bool is_die() const { return die_; }

		static int64_t numCreated() {
			int64_t v = 0;
			s_numCreated_.compare_exchange_strong(v, 0);
			return v;
		}

	private:
		///回调函数
		const TimerCallback callback_;
		///超时时间（绝对时间）
		CTimestamp expiration_;
		///间隔多久重新闹铃 (秒)
		const double interval_;
		///是否重复
		const bool repeat_;
		///Timer序号
		const int64_t sequence_;
		///无效
		bool die_;
		///创建Timer序号使用，static
		//static AtomicInt64 s_numCreated_;
		static std::atomic<int64_t> s_numCreated_;
	};
}

#endif	//__CRITSEC_H__
