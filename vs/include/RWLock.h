#ifndef __SYS_UTILITIES_WRLOCK_H__
#define __SYS_UTILITIES_WRLOCK_H__

#include "SysUtilities.h"
#include "CritSec.h"
#include "Sem.h"

namespace SYS_UTL
{
	/**
	* \brief 读写锁封装类.
	*/
	class SYS_UTL_CPPAPI CRWLock
	{
		CRWLock(CRWLock const &) = delete;
		CRWLock(CRWLock &&) = delete;
		CRWLock& operator= (CRWLock const &) = delete;
		CRWLock& operator= (CRWLock &&) = delete;
	public:
		CRWLock();
		~CRWLock();
		/**
		* \brief 加读锁
		* \return bool 被激活返回 true 其他情况返回 false
		*/
		bool RLock();
		/**
		* \brief 尝试加读锁
		* \param uiTimeOut 超时时间(毫秒)
		* \return bool 被激活返回 true 其他情况返回 false
		*/
		bool TryRLock(DWORD dwTimeOut = 0);
		/**
		* \brief 释放读锁
		*/
		void RUnLock();
		/**
		* \brief 加写锁
		* \return bool 被激活返回 true 其他情况返回 false
		*/
		bool WLock();
		/**
		* \brief 尝试加写锁
		* \return bool 被激活返回 true 其他情况返回 false
		*/
		bool TryWLock(DWORD dwTimeOut = 0);
		/**
		* \brief 释放写锁
		*/
		void WUnLock();
	private:
		void __Init();
		void __UnInit();
		bool __IsInit() const;
	private:
		/**< 释放初始化.*/
		bool m_Init;
		/**< 信号量.*/
		SYS_UTL::CSem m_Sem;
		/**< 读者数量.*/
		unsigned int m_uiReaders;
		/**< 读.临界资源.*/
		SYS_UTL::CCritSec m_Sec;
	};
}

#endif	//__AUTOLOCK_H__
