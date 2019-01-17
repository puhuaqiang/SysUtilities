#include "../SysUtilities/stdafx.h"
#include "../include/RWLock.h"
#include "../include/AutoLock.h"

namespace SYS_UTL
{
	CRWLock::CRWLock() :
		m_Init(false),
		m_uiReaders(0)
	{
		__Init();
	}

	CRWLock::~CRWLock()
	{
		__UnInit();
	}

	void CRWLock::__Init()
	{
		if (__IsInit())
		{
			return;
		}
		/* Initialize the semaphore that acts as the write lock. */
		if (!m_Sem.Init(1, 1))
		{
			DBG_ERROR;
			return;
		}

		/* Initialize the reader count. */
		m_uiReaders = 0;
		m_Init = true;
	}

	void CRWLock::__UnInit()
	{
		if (!__IsInit())
		{
			return;
		}
		m_Sem.UnInit();
		m_Init = false;
	}

	bool CRWLock::__IsInit() const
	{
		return m_Init;
	}

	bool CRWLock::RLock()
	{
		if (!__IsInit())
		{
			DBG_ERROR;
			return false;
		}
		/* *
		* Acquire the lock that protects the reader count. 
		* Release the lock that protects the reader count.
		*/
		CAutoLock lck(&m_Sec);
		bool lock = true;

		/* Increase the reader count, and lock for write if this is the first reader.
		* 增加读取器数量，如果这是第一个读取器，则锁定写入
		*/
		if (++m_uiReaders == 1) {
			if (!m_Sem.Wait())
			{
				lock = false;
				m_uiReaders--;
			}
		}
		return lock;
	}

	bool CRWLock::TryRLock(DWORD dwTimeOut /*= 0*/)
	{
		if (!__IsInit())
		{
			DBG_ERROR;
			return false;
		}
		CAutoLock lck(&m_Sec, SYS_UTL::LOCK_FLAG::lock_defer);
		if (!lck.Lock(dwTimeOut))
		{
			return false;
		}

		bool lock = false;
		if (m_uiReaders == 0) {
			/* Currently there are no other readers, which means that the write lock
			* needs to be acquired.
			* 目前没有其他读取器，这意味着需要获取写锁
			*/
			if (m_Sem.TryWait(dwTimeOut))
			{
				lock = true;
				m_uiReaders++;
			}
		}
		else {
			/* The write lock has already been acquired because there are other
			* active readers.
			* 已经获取了写锁定，因为还有其他活动读取器。
			*/
			m_uiReaders++;
			lock = true;
		}
		return lock;
	}

	void CRWLock::RUnLock()
	{
		if (!__IsInit())
		{
			DBG_ERROR;
			return;
		}
		CAutoLock lck(&m_Sec);

		if (--m_uiReaders == 0) {
			m_Sem.Post();
		}
	}

	bool CRWLock::WLock()
	{
		if (!__IsInit())
		{
			DBG_ERROR;
			return false;
		}
		if (!m_Sem.Wait())
		{
			return false;
		}
		return true;
	}

	bool CRWLock::TryWLock(DWORD dwTimeOut /*= 0*/)
	{
		if (!__IsInit())
		{
			DBG_ERROR;
			return false;
		}
		if (!m_Sem.TryWait(dwTimeOut))
		{
			return false;
		}
		return true;
	}

	void CRWLock::WUnLock()
	{
		if (!__IsInit())
		{
			DBG_ERROR;
			return;
		}
		m_Sem.Post();
	}

}
