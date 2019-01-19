#include "../SysUtilities/stdafx.h"
#include "../include/Event.h"

namespace SYS_UTL
{
	CEventWrap::CEventWrap()
	{
		m_hEvent = NULL;
	};

	CEventWrap::~CEventWrap()
	{
		UnInit();
	};

	bool CEventWrap::Init(BOOL bManualReset, BOOL bInitialState, const char* lpName, bool bOpen/* = false*/)
	{
		return Init(NULL, bManualReset, bInitialState, lpName, bOpen);
	}

	bool CEventWrap::Init(LPSECURITY_ATTRIBUTES lpSec, BOOL bManualReset, BOOL bInitialState, const char* lpName, bool bOpen /*= false*/)
	{
		if (bOpen)
		{
			if (NULL == lpName)
			{
				return false;
			}
			m_hEvent = OpenEvent(EVENT_ALL_ACCESS, TRUE, lpName);
		}
		else{
			m_hEvent = CreateEvent(lpSec, bManualReset, bInitialState, lpName);
		}
		return __Valid();
	}

	void CEventWrap::UnInit()
	{
		if (!__Valid())
		{
			DBG_ERROR;
			return;
		}
		CloseHandle(m_hEvent);
		m_hEvent = NULL;
	}

	bool CEventWrap::Wait()
	{
		if (!__Valid())
		{
			DBG_ERROR;
			return false;
		}
		if (WaitForSingleObject(m_hEvent, INFINITE) != WAIT_OBJECT_0)
		{
			return false;
		}
		return true;
	};

	bool CEventWrap::TryWait(DWORD dwTimeOut /*= 500*/)
	{
		if (!__Valid())
		{
			DBG_ERROR;
			return false;
		}
		if (WaitForSingleObject(m_hEvent, dwTimeOut) != WAIT_OBJECT_0)
		{
			return false;
		}
		return true;
	}

	bool CEventWrap::Set()
	{
		if (!__Valid())
		{
			DBG_ERROR;
			return false;
		}
		if (!SetEvent(m_hEvent))
		{
			DBG_ERROR;
			return false;
		}
		return true;
	}

	bool CEventWrap::__Valid() const
	{
		return m_hEvent != NULL;
	}

	bool CEventWrap::ReSet()
	{
		if (!__Valid())
		{
			DBG_ERROR;
			return false;
		}
		if (!ResetEvent(m_hEvent))
		{
			DBG_ERROR;
			return false;
		}
		return true;
	}

	bool CEventWrap::Pulse()
	{
		if (!__Valid())
		{
			DBG_ERROR;
			return false;
		}
		if (!PulseEvent(m_hEvent))
		{
			DBG_ERROR;
			return false;
		}
		return true;
	}

	HANDLE CEventWrap::Get()
	{
		return m_hEvent;
	}

}
