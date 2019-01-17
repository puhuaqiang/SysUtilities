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
		* ���Ӷ�ȡ��������������ǵ�һ����ȡ����������д��
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
			* Ŀǰû��������ȡ��������ζ����Ҫ��ȡд��
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
			* �Ѿ���ȡ��д��������Ϊ�����������ȡ����
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
