#ifndef __SYS_UTILITIES_REPLYLOCK_H__
#define __SYS_UTILITIES_REPLYLOCK_H__

#include "SysUtilities.h"
#include "Cond.h"

namespace SYS_UTL
{
	/**
	* \brief 特殊的读写锁..
	*	一个线程加锁，执行命令，然后等待某个条件触发
	*	一个线程得到命令执行结果，触发条件.
	*	典型应用场景：网络通信中，网络命令互斥执行，等待命令执行完成。
	*
	*	自动加锁释放锁 [CAutoRepLock]
	*/
	class SYS_UTL_CPPAPI CReplyLock
	{
		CReplyLock(CReplyLock const &) = delete;
		CReplyLock(CReplyLock &&) = delete;
		CReplyLock& operator= (CReplyLock const &) = delete;
		CReplyLock& operator= (CReplyLock &&) = delete;
	public:
		CReplyLock();
		~CReplyLock();
		/**
		* \brief 加锁
		*/
		bool Lock();
		/**
		* \brief 加锁
		* \param uiTimeOut 超时时间(毫秒)
		*/
		bool TryLock(DWORD dwTimeOut = 100);
		/**
		* \brief 释放锁
		*/
		void UnLock();

		/**
		* \brief 触发条件变量
		*	调用 Signal 前,不能 Lock.	
		*/
		int Signal();

		/**
		* \brief 等待执行回复
		*	只有在 Lock 成功后，才能 Wait
		*/
		bool Wait();
		/**
		* \brief 等待执行回复
		*	只有在 Lock 成功后，才能 TryWait
		* \param uiTimeOut 超时时间(毫秒)
		*/
		bool TryWait(DWORD dwTimeOut = 100);

	private:
		/**< 信号量.*/
		SYS_UTL::CCond m_Cond;
		/**< 临界资源.*/
		SYS_UTL::CCritSec m_Sec;

	};
}

#endif	//__AUTOLOCK_H__
