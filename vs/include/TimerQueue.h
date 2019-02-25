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
		* @param when ��ʱ������ʱ��
		* @param interval ���ʱ�� >0 �����Լ�ʱ��. ��λ����
		*/
		CTimerId addTimer(const TimerCallback& cb, CTimestamp when, double seconds);
		void cancel(CTimerId timerId);
		SYS_UTL::CCritSec mutex_;
	};

	/**
	��װ�˶�ʱ����ص��¼��ͻص�����
	Timer��װ�˶�ʱ����һЩ���������糬ʱ�ص���������ʱʱ�䡢��ʱ���Ƿ��ظ����ظ����ʱ�䡢��ʱ�������кš�
	�亯������������Щ������run()�������ûص�������restart()����������ʱ�����������Ϊ�ظ���
	*/
	class SYS_UTL_CPPAPI CTimer : CNonCopyAble
	{
	public:
		CTimer(TimerCallback cb, CTimestamp when, double seconds)
			: callback_(std::move(cb)),//�ص�����
			expiration_(when),//��ʱʱ��
			interval_(seconds),//����ظ������ʱ��
			repeat_(seconds > 0.0),//�Ƿ��ظ�
			die_(false),
			sequence_(s_numCreated_++)//��ǰ��ʱ�������к�
		{ }

		//��ʱʱ���ûص�����
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
		///�ص�����
		const TimerCallback callback_;
		///��ʱʱ�䣨����ʱ�䣩
		CTimestamp expiration_;
		///�������������� (��)
		const double interval_;
		///�Ƿ��ظ�
		const bool repeat_;
		///Timer���
		const int64_t sequence_;
		///��Ч
		bool die_;
		///����Timer���ʹ�ã�static
		//static AtomicInt64 s_numCreated_;
		static std::atomic<int64_t> s_numCreated_;
	};
}

#endif	//__CRITSEC_H__
