#ifndef __SYS_UTILITIES_SEMAPHORE_H__
#define __SYS_UTILITIES_SEMAPHORE_H__

#include "SysUtilities.h"

namespace SYS_UTL
{
	/***
	* \brief 信号量封装类
	*	1.支持超时返回.
	*	2.支持非加锁线程通过激活标致,通知已加锁线程释放锁。
	*/
	class SYS_UTL_CPPAPI CSem
	{
		CSem(const CSem &refAutoLock);
		CSem &operator=(const CSem &refAutoLock);
	public:
		CSem();
		~CSem();
		/**
		* \brief 初始化.
		*/
		bool Init(unsigned int uiSem, unsigned int uiMax, const char* lpName = NULL, bool bOpen = false);
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
		*	返回即表示触发信号成功
		*/
		bool Post();

	private:
		bool __Valid() const;
	protected:
		/**信号量*/
		HANDLE m_hSem;
	};
}

#endif	//__CRITSEC_H__
