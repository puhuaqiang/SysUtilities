#include "../SysUtilities/stdafx.h"
#include "../include/Mutex.h"

namespace SYS_UTL
{
	CMutexWrap::CMutexWrap()
	{
		m_hMutex = NULL;
	};

	CMutexWrap::~CMutexWrap()
	{
		UnInit();
	};

	bool CMutexWrap::Init(BOOL bInitialOwner, const char* lpName, bool bOpen/* = false*/)
	{
		return Init(NULL, bInitialOwner, lpName, bOpen);
	}

	bool CMutexWrap::Init(LPSECURITY_ATTRIBUTES lpSec, BOOL bInitialOwner, const char* lpName, bool bOpen/* = false*/)
	{
		if (bOpen)
		{
			if (NULL == lpName)
			{
				return false;
			}
			m_hMutex = OpenEvent(MUTEX_ALL_ACCESS, TRUE, lpName);
		}
		else{
			m_hMutex = CreateMutex(lpSec, bInitialOwner, lpName);
		}
		return __Valid();
	}

	void CMutexWrap::UnInit()
	{
		if (!__Valid())
		{
			DBG_ERROR;
			return;
		}
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}

	bool CMutexWrap::Wait()
	{
		if (!__Valid())
		{
			DBG_ERROR;
			return false;
		}
		if (WaitForSingleObject(m_hMutex, INFINITE) != WAIT_OBJECT_0)
		{
			return false;
		}
		return true;
	};

	bool CMutexWrap::TryWait(DWORD dwTimeOut /*= 500*/)
	{
		if (!__Valid())
		{
			DBG_ERROR;
			return false;
		}
		if (WaitForSingleObject(m_hMutex, dwTimeOut) != WAIT_OBJECT_0)
		{
			return false;
		}
		return true;
	}

	bool CMutexWrap::Release()
	{
		if (!__Valid())
		{
			DBG_ERROR;
			return false;
		}
		if (!ReleaseMutex(m_hMutex))
		{
			DBG_ERROR;
			return false;
		}
		return true;
	}

	HANDLE CMutexWrap::Get()
	{
		return m_hMutex;
	}

	bool CMutexWrap::__Valid() const
	{
		return m_hMutex != NULL;
	}

	CAutoMutex::CAutoMutex(SYS_UTL::CMutexWrap *lpMutex)
	{
		m_owns = FALSE;
		m_lpMutex = lpMutex;
		assert(NULL != lpMutex);
		if (m_lpMutex->Wait())
		{
			m_owns = TRUE;
		}
	}

	CAutoMutex::CAutoMutex(SYS_UTL::CMutexWrap *lpMutex, SYS_UTL::LOCK_FLAG::__adopt_lock_t)
	{
		m_owns = FALSE;
		m_lpMutex = lpMutex;
		assert(NULL != lpMutex);
		if (m_lpMutex->Wait())
		{
			m_owns = TRUE;
		}
	}

	CAutoMutex::CAutoMutex(SYS_UTL::CMutexWrap *lpMutex, SYS_UTL::LOCK_FLAG::__defer_lock_t)
	{
		m_owns = FALSE;
		m_lpMutex = lpMutex;
		assert(NULL != lpMutex);
	}

	CAutoMutex::~CAutoMutex()
	{
		if (Owns())
		{
			m_lpMutex->Release();
		}
	}

	BOOL CAutoMutex::Owns() const
	{
		return m_owns;
	}

	BOOL CAutoMutex::Wait()
	{
		if (Owns())
		{
			return TRUE;
		}
		if (m_lpMutex->Wait())
		{
			m_owns = TRUE;
		}
		return Owns();
	}

	BOOL CAutoMutex::TryWait(DWORD dwTimeOut /*= 100*/)
	{
		if (Owns())
		{
			return TRUE;
		}
		if (m_lpMutex->TryWait(dwTimeOut))
		{
			m_owns = TRUE;
		}
		return Owns();
	}

}
