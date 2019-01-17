#include "../SysUtilities/stdafx.h"
#include "../include/CritSec.h"

namespace SYS_UTL
{
	CCritSec::CCritSec()
	{
		InitializeCriticalSection(&m_CritSec);
	};

	CCritSec::~CCritSec()
	{
		DeleteCriticalSection(&m_CritSec);
	};

	void CCritSec::Lock()
	{
		EnterCriticalSection(&m_CritSec);
	};

	bool CCritSec::LockWait(int iTimeOut /*= 500*/)
	{
		BOOL bTrySuccess = FALSE;
		DWORD dwCut = GetTickCount();
		do
		{
			bTrySuccess = TryEnterCriticalSection(&m_CritSec);
			if (bTrySuccess)
			{
				break;
			}
			if (abs((int)(GetTickCount() - dwCut)) >= iTimeOut)
			{
				break;
			}
			Sleep(10);
		} while (true);
		return bTrySuccess ? true : false;
	};

	bool CCritSec::LockWaitEx(bool& bWait, int iTimeOut /*= 500*/)
	{
		BOOL bTrySuccess = FALSE;
		DWORD dwCut = GetTickCount();
		do
		{
			bTrySuccess = TryEnterCriticalSection(&m_CritSec);
			if (bTrySuccess)
			{
				break;
			}
			if (abs((int)(GetTickCount() - dwCut)) >= iTimeOut)
			{
				break;
			}
			Sleep(10);
		} while (bWait);
		return bTrySuccess ? true : false;
	};

	void CCritSec::Unlock()
	{
		LeaveCriticalSection(&m_CritSec);
	};

	bool CCritSec::RequireLock()
	{
		if (m_bUrgentNeed)
		{
			return FALSE;
		}
		m_bUrgentNeed = TRUE;
		return TRUE;
	};
	void CCritSec::ReleaseRequireFlag()
	{
		m_bUrgentNeed = FALSE;
	};
	bool CCritSec::IsRequire()
	{
		return m_bUrgentNeed ? true : false;
	};
}
