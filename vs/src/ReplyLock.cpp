#include "../SysUtilities/stdafx.h"
#include "../include/ReplyLock.h"
#include "../include/AutoLock.h"

namespace SYS_UTL
{
	CReplyLock::CReplyLock() 
	{
	}

	CReplyLock::~CReplyLock()
	{
	}

	bool CReplyLock::Lock()
	{
		m_Sec.Lock();
		return true;
	}

	bool CReplyLock::TryLock(DWORD dwTimeOut /*= 100*/)
	{
		return m_Sec.LockWait(dwTimeOut);
	}

	void CReplyLock::UnLock()
	{
		m_Sec.Unlock();
	}

	int CReplyLock::Signal()
	{
		return m_Cond.Signal();
	}

	bool CReplyLock::Wait()
	{
		m_Cond.Wait();
		return true;
	}

	bool CReplyLock::TryWait(DWORD dwTimeOut /*= 100*/)
	{
		return m_Cond.TryWait(dwTimeOut);;
	}
}
