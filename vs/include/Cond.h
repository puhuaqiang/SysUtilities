#ifndef __SYS_UTILITIES_CONDITION_H__
#define __SYS_UTILITIES_CONDITION_H__

#include "CritSec.h"
#include "Sem.h"

namespace SYS_UTL
{
	/**
	* \brief 等到指定数目的线程一起到达才能一起继续执行
	*/
	class SYS_UTL_CPPAPI CCond
	{
		CCond(const CCond &ref);
		CCond &operator=(const CCond &ref);

	public:
		CCond();
		~CCond();
		/**
		* \brief 初始化
		*/
		int Init();
		/**
		* \brief 释放资源
		*/
		void UnInit();

		/**
		* 触发条件变量,唤醒一个等待中的线程
		*/
		int Signal();

		/**
		* 触发条件变量,唤醒等待中的所有线程
		*/
		int Broadcast();

		/**
		* 唤醒条件变量被触发
		*/
		bool Wait();

		/**
		* 唤醒条件变量被触发
		* /param dwTimeOut 超时时间(毫秒)
		*/
		bool TryWait(DWORD dwTimeOut);
		/**
		* \brief 是否初始化成功
		*/
		BOOL IsInit() const;

		typedef union _COND_PROPERTY
		{
			CONDITION_VARIABLE cond_var;
			struct {
				unsigned int waiters_count;
				CRITICAL_SECTION waiters_count_lock;
				HANDLE signal_event;
				HANDLE broadcast_event;
			} fallback;
		}COND_PROPERTY, *LPLCOND_PROPERTY;

	private:
		/// 是否初始化成功
		BOOL m_bInit;
		SYS_UTL::CCritSec m_Mutex;
		COND_PROPERTY m_Cond;
	};

}

#endif	//__AUTOLOCK_H__
