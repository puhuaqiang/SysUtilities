#ifndef __SYS_UTILITIES_MUTEX_H__
#define __SYS_UTILITIES_MUTEX_H__

#include "SysUtilities.h"
#include "AutoLock.h"

namespace SYS_UTL
{
	/***
	* \brief 事件封装类
	*/
	class SYS_UTL_CPPAPI CMutexWrap
	{
		CMutexWrap(const CMutexWrap &ref);
		CMutexWrap &operator=(const CMutexWrap &ref);
	public:
		CMutexWrap();
		~CMutexWrap();
		/**
		* \brief 初始化.
		* \param bManualReset 手动置位还是自动置位，传入TRUE表示手动置位，传入FALSE表示自动置位。
		* \param bInitialState 事件的初始状态，传入TRUR表示已触发。
		* \param lpName 事件的名称，传入NULL表示匿名事件
		* \param bOpen 是否打开已存在的事件
		*/
		bool Init(BOOL bInitialOwner, const char* lpName, bool bOpen = false);
		bool Init(LPSECURITY_ATTRIBUTES, BOOL bInitialOwner, const char* lpName, bool bOpen = false);
		/**
		* \brief 释放资源.
		*/
		void UnInit();

		/**
		* \brief 等待信号.
		*	返回 true 即表示信号被触发, 否则失败.
		*/
		bool Wait();

		/**
		* \brief 等待信号.
		* \param iTimeOut 超时时间(毫秒)
		* \return 返回TRUE 表示加锁成功,否则加锁失败
		*/
		bool TryWait(DWORD dwTimeOut = 500);
	
		/**
		* \brief 触发信号.
		*	返回即表示触发成功
		*/
		bool Release();

		HANDLE Get();
	private:
		bool __Valid() const;
	protected:
		/**信号量*/
		HANDLE m_hMutex;
	};

	class SYS_UTL_CPPAPI CAutoMutex
	{
		CAutoMutex(const CAutoMutex &ref);
		CAutoMutex &operator=(const CAutoMutex &ref);

	public:
		/**
		* \brief 
		*/
		CAutoMutex(SYS_UTL::CMutexWrap *lpMutex);
		/**
		* \brief 
		*/
		CAutoMutex(SYS_UTL::CMutexWrap *lpMutex, SYS_UTL::LOCK_FLAG::__adopt_lock_t);
		CAutoMutex(SYS_UTL::CMutexWrap *lpMutex, SYS_UTL::LOCK_FLAG::__defer_lock_t);

		~CAutoMutex();

		/**
		* \brief 是否加锁成功.
		* \return 返回TRUE,加锁成功, 否则加锁失败.
		*/
		BOOL Owns() const;

		/**
		* \brief 等待执行回复
		*	只有在 Lock 成功后，才能 Wait
		*/
		BOOL Wait();
		/**
		* \brief 等待执行回复
		*	只有在 Lock 成功后，才能 TryWait
		* \param uiTimeOut 超时时间(毫秒)
		*/
		BOOL TryWait(DWORD dwTimeOut = 100);

	private:
		SYS_UTL::CMutexWrap* m_lpMutex;
		/**是否拥有锁*/
		BOOL m_owns;
	};
}

#endif	//__CRITSEC_H__
