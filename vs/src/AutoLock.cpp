#include "../SysUtilities/stdafx.h"
#include "../include/AutoLock.h"

namespace SYS_UTL
{
	CAutoLock::CAutoLock(SYS_UTL::CCritSec *plock)
	{
		__Initialize();
		m_lpLock = plock;
		assert(NULL != plock);
		m_lpLock->Lock();
		m_owns = TRUE;
	}
	CAutoLock::CAutoLock(SYS_UTL::CCritSec *plock, SYS_UTL::LOCK_FLAG::__adopt_lock_t)
	{
		__Initialize();
		m_lpLock = plock;
		assert(NULL != plock);
		m_lpLock->Lock();
		m_owns = TRUE;
	}
	CAutoLock::CAutoLock(SYS_UTL::CCritSec *plock, SYS_UTL::LOCK_FLAG::__defer_lock_t)
	{
		__Initialize();
		m_lpLock = plock;
		assert(NULL != plock);
	}
	CAutoLock::CAutoLock(SYS_UTL::CCritSec *plock, SYS_UTL::LOCK_FLAG::__defer_req_lock_t)
	{
		__Initialize();
		m_lpLock = plock;
		assert(NULL != plock);
		m_bRequire = TRUE;
		if (m_lpLock->RequireLock())
		{//激活成功.
			m_bRequireSucc = TRUE;
		}
	}
	CAutoLock::~CAutoLock()
	{
		if (m_owns)
		{
			m_lpLock->Unlock();
			m_owns = FALSE;
		}

		if (m_bRequire && m_bRequireSucc)
		{
			/**
			* 如果是当前对象激活的强制加锁标识,则需要释放
			*/
			m_lpLock->ReleaseRequireFlag();
		}
		m_bRequire = FALSE;
		m_bRequireSucc = FALSE;
	}

	BOOL CAutoLock::Owns() const
	{
		return m_owns;
	}

	BOOL CAutoLock::Lock()
	{
		if (Owns())
		{
			return TRUE;
		}
		if (!__CheckRequireFlag())
		{
			DBG_E;
			return FALSE;
		}
		m_lpLock->Lock();
		m_owns = TRUE;
		return TRUE;
	}

	BOOL CAutoLock::Lock(DWORD dwTimeOut)
	{
		if (Owns())
		{
			return TRUE;
		}
		if (!__CheckRequireFlag())
		{
			DBG_E;
			return FALSE;
		}
		if (m_lpLock->LockWait(dwTimeOut))
		{
			m_owns = TRUE;
		}
		return m_owns;
	}

	BOOL CAutoLock::IsOtherRequire() const
	{
		return m_lpLock->IsRequire();
	}

	BOOL CAutoLock::IsRequire() const
	{
		return m_bRequire;
	}

	BOOL CAutoLock::IsRequireSucc() const
	{
		return m_bRequireSucc;
	}

	void CAutoLock::__Initialize()
	{
		m_lpLock = NULL;
		m_owns = FALSE;
		m_bRequire = FALSE;
		m_bRequireSucc = FALSE;
	}
	BOOL CAutoLock::__CheckRequireFlag()
	{
		if (!IsRequire())
		{//该实例未要求[强制加锁标识]
			return TRUE;
		}
		if (!IsRequireSucc())
		{//该实例没有请求[强制加锁标识]成功,该标识已经被其他线程请求了
			DBG_I("autolock [IsRequireSucc] fail");
			return FALSE;
		}
		return TRUE;
	}

	CAutoRWLock::CAutoRWLock(SYS_UTL::CRWLock *plock, SYS_UTL::LOCK_FLAG::__adopt_read_lock_t)
	{
		m_owns = FALSE;
		m_lpLock = plock;
		assert(NULL != m_lpLock);
		if (m_lpLock->RLock())
		{
			m_owns = TRUE;
		}
		m_nLockType = RWLOCK_TYPE_READ;
	}

	CAutoRWLock::CAutoRWLock(SYS_UTL::CRWLock *plock, SYS_UTL::LOCK_FLAG::__adopt_write_lock_t)
	{
		m_owns = FALSE;
		m_lpLock = plock;
		assert(NULL != m_lpLock);
		if (m_lpLock->WLock())
		{
			m_owns = TRUE;
		}
		m_nLockType = RWLOCK_TYPE_WRITE;
	}

	CAutoRWLock::CAutoRWLock(SYS_UTL::CRWLock *plock, SYS_UTL::LOCK_FLAG::__defer_read_lock_t)
	{
		m_owns = FALSE;
		m_lpLock = plock;
		assert(NULL != m_lpLock);
		m_nLockType = RWLOCK_TYPE_READ;
	}

	CAutoRWLock::CAutoRWLock(SYS_UTL::CRWLock *plock, SYS_UTL::LOCK_FLAG::__defer_write_lock_t)
	{
		m_owns = FALSE;
		m_lpLock = plock;
		assert(NULL != m_lpLock);
		m_nLockType = RWLOCK_TYPE_WRITE;
	}

	CAutoRWLock::~CAutoRWLock()
	{
		if (Owns())
		{
			if (RWLOCK_TYPE_READ == m_nLockType)
			{
				m_lpLock->RUnLock();
			}
			else if (RWLOCK_TYPE_WRITE == m_nLockType)
			{
				m_lpLock->WUnLock();
			}
		}
	}

	BOOL CAutoRWLock::Owns() const
	{
		return m_owns;
	}

	BOOL CAutoRWLock::Lock()
	{
		if (Owns())
		{
			return TRUE;
		}
		if (RWLOCK_TYPE_READ == m_nLockType)
		{
			if (m_lpLock->RLock())
			{
				m_owns = TRUE;
			}
		}
		else if (RWLOCK_TYPE_WRITE == m_nLockType)
		{
			if (m_lpLock->WLock())
			{
				m_owns = TRUE;
			}
		}
		else{
			DBG_E;
			return FALSE;
		}
		return Owns();
	}

	BOOL CAutoRWLock::Lock(DWORD dwTimeOut)
	{
		if (Owns())
		{
			return TRUE;
		}
		if (RWLOCK_TYPE_READ == m_nLockType)
		{
			if (m_lpLock->TryRLock(dwTimeOut))
			{
				m_owns = TRUE;
			}
		}
		else if (RWLOCK_TYPE_WRITE == m_nLockType)
		{
			if (m_lpLock->TryWLock(dwTimeOut))
			{
				m_owns = TRUE;
			}
		}
		else{
			DBG_E;
			return FALSE;
		}
		return Owns();
	}

	CAutoRepLock::CAutoRepLock(SYS_UTL::CReplyLock *plock, SYS_UTL::LOCK_FLAG::__adopt_lock_t)
	{
		m_owns = FALSE;
		m_lpLock = plock;
		assert(NULL != m_lpLock);
		m_lpLock->Lock();
		m_owns = TRUE;
	}

	CAutoRepLock::CAutoRepLock(SYS_UTL::CReplyLock *plock, SYS_UTL::LOCK_FLAG::__defer_lock_t)
	{
		m_owns = FALSE;
		m_lpLock = plock;
		assert(NULL != m_lpLock);
	}

	CAutoRepLock::~CAutoRepLock()
	{
		if (Owns())
		{
			m_lpLock->UnLock();
		}
	}

	BOOL CAutoRepLock::Owns() const
	{
		return m_owns;
	}

	BOOL CAutoRepLock::Lock()
	{
		if (Owns())
		{
			return TRUE;
		}
		m_owns = m_lpLock->Lock() ? TRUE : FALSE;
		return Owns();
	}

	BOOL CAutoRepLock::Lock(DWORD dwTimeOut)
	{
		if (Owns())
		{
			return TRUE;
		}
		m_owns = m_lpLock->TryLock(dwTimeOut) ? TRUE : FALSE;
		return Owns();
	}

	BOOL CAutoRepLock::Wait()
	{
		if (!Owns())
		{
			DBG_E;
			return FALSE;
		}
		return m_lpLock->Wait() ? TRUE : FALSE;
	}

	BOOL CAutoRepLock::TryWait(DWORD dwTimeOut /*= 100*/)
	{
		if (!Owns())
		{
			DBG_E;
			return FALSE;
		}
		return m_lpLock->TryWait(dwTimeOut) ? TRUE : FALSE;
	}

}