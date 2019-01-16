#ifndef __SYS_UTILITIES_CRITSEC_H__
#define __SYS_UTILITIES_CRITSEC_H__

#include "SysUtilities.h"

namespace SYS_UTL
{
	/***
	* \brief 临界区封装类
	*	1.支持超时返回.
	*	2.支持非加锁线程通过激活标致,通知已加锁线程释放锁。
	*/
	class SYS_UTL_CPPAPI CCritSec
	{
		CCritSec(const CCritSec &refAutoLock);
		CCritSec &operator=(const CCritSec &refAutoLock);
	public:
		CCritSec();
		~CCritSec();
		/**
		* \brief 加锁.
		*	返回即表示加锁成功
		*/
		void Lock();

		/**
		* \brief 加锁.
		* \param iTimeOut 超时时间
		* \return 返回TRUE 表示加锁成功,否则加锁失败
		*/
		bool LockWait(int iTimeOut = 500);
		/**
		* \brief 加锁.
		* \param bWait 循环等待标识
		* \param iTimeOut 超时时间
		* \return 返回TRUE 表示加锁成功,否则加锁失败
		*/
		bool LockWaitEx(bool& bWait, int iTimeOut = 500);
		/**
		* \brief 释放锁.
		*	返回即表示释放锁成功
		*/
		void Unlock();

		/**
		* \brief 要求立刻加锁.该接口需要在加锁前调用
		*	该接口必须与 [ReleaseRequireFlag]配对使用,
		*	谁(线程) [RequireLock] 成功就需要调用 [ReleaseRequireFlag]释放
		* \return 如果返回 FALSE,标识请求失败,说明已经被某个线程标识了
		*/
		bool RequireLock();
		/**
		* \brief 释放 强制加锁标识
		*/
		void ReleaseRequireFlag();
		/**
		* \brief 强制加锁标识 是否被设置
		*	非紧急线程判断到标识被设置则需要立刻释放锁.
		*/
		bool IsRequire();

	protected:
		/**临界区实例*/
		CRITICAL_SECTION m_CritSec;
		/**
		* \brief 某线程急需锁,比如界面线程等.
		*	用来标识线程已经强制锁,
		*	其他非紧急线程加锁后,需要判断该标识,如果该标识被设置,则非紧急线程需要立刻释放锁.
		*/
		BOOL m_bUrgentNeed;
	};
}

#endif	//__CRITSEC_H__
