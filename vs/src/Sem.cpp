#include "../SysUtilities/stdafx.h"
#include "../include/sem.h"

namespace SYS_UTL
{
	CSem::CSem()
	{
		m_hSem = NULL;
	};

	CSem::~CSem()
	{
		UnInit();
	};

	bool CSem::Init(unsigned int uiSem, unsigned int uiMax, const char* lpName /*= NULL*/, bool bOpen /*= false*/)
	{
		if (bOpen)
		{
			if (NULL == lpName)
			{
				return false;
			}
			m_hSem = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, lpName);
		}
		else{
			m_hSem = CreateSemaphore(NULL, uiSem, uiMax, lpName);
		}
		return __Valid();
	}

	void CSem::UnInit()
	{
		if (!__Valid())
		{
			DBG_ERROR;
			return;
		}
		CloseHandle(m_hSem);
		m_hSem = NULL;
	}

	bool CSem::Wait()
	{
		if (!__Valid())
		{
			DBG_ERROR;
			return false;
		}
		if (WaitForSingleObject(m_hSem, INFINITE) != WAIT_OBJECT_0)
		{
			return false;
		}
		return true;
	};

	bool CSem::TryWait(DWORD dwTimeOut /*= 500*/)
	{
		if (!__Valid())
		{
			DBG_ERROR;
			return false;
		}
		if (WaitForSingleObject(m_hSem, dwTimeOut) != WAIT_OBJECT_0)
		{
			return false;
		}
		return true;
	}

	bool CSem::Post()
	{
		if (!__Valid())
		{
			DBG_ERROR;
			return false;
		}
		if (!ReleaseSemaphore(m_hSem, 1, NULL))
		{
			DBG_ERROR;
			return false;
		}
		return true;
	}

	bool CSem::__Valid() const
	{
		return m_hSem != NULL;
	}
}
