#include "../SysUtilities/stdafx.h"
#include "../include/Cond.h"
#include "../include/AutoLock.h"

#define HAVE_CONDVAR_API() (pInitializeConditionVariable != NULL)


static int __cond_condvar_init(CONDITION_VARIABLE& cond)
{
	pInitializeConditionVariable(&cond);
	return 0;
}

static void __cond_condvar_destroy(CONDITION_VARIABLE& cond)
{
	/* nothing to do */
}

static void __cond_condvar_signal(CONDITION_VARIABLE& cond)
{
	if (NULL == pWakeConditionVariable)
	{
		DBG_ERROR;
		return;
	}
	pWakeConditionVariable(&cond);
}

static void __cond_condvar_broadcast(CONDITION_VARIABLE& cond)
{
	if (NULL == pWakeAllConditionVariable)
	{
		DBG_ERROR;
		return;
	}
	pWakeAllConditionVariable(&cond);
}

static void __cond_condvar_wait(CONDITION_VARIABLE& cond, CRITICAL_SECTION& mutex)
{
	if (!pSleepConditionVariableCS(&cond, &mutex, INFINITE))
		abort();
}

static int __cond_condvar_timedwait(CONDITION_VARIABLE& cond,
	CRITICAL_SECTION& mutex, DWORD timeout) 
{
	if (pSleepConditionVariableCS(&cond, &mutex, timeout))
	{
		return 0;
	}
	if (GetLastError() != ERROR_TIMEOUT)
	{
		DBG_ERROR;
		//abort();
	}
	return -1;
}


/* This condition variable implementation is based on the SetEvent solution
* (section 3.2) at http://www.cs.wustl.edu/~schmidt/win32-cv-1.html
* We could not use the SignalObjectAndWait solution (section 3.4) because
* it want the 2nd argument (type uv_mutex_t) of uv_cond_wait() and
* uv_cond_timedwait() to be HANDLEs, but we use CRITICAL_SECTIONs.
*/

static int __cond_fallback_init(SYS_UTL::CCond::LPLCOND_PROPERTY cond) {
	int err = 0;

	/* Initialize the count to 0. */
	cond->fallback.waiters_count = 0;

	InitializeCriticalSection(&cond->fallback.waiters_count_lock);

	/* Create an auto-reset event. */
	cond->fallback.signal_event = CreateEvent(NULL,  /* no security */
		FALSE, /* auto-reset event */
		FALSE, /* non-signaled initially */
		NULL); /* unnamed */
	if (!cond->fallback.signal_event) {
		err = GetLastError();
		goto error2;
	}

	/* Create a manual-reset event. */
	cond->fallback.broadcast_event = CreateEvent(NULL,  /* no security */
		TRUE,  /* manual-reset */
		FALSE, /* non-signaled */
		NULL); /* unnamed */
	if (!cond->fallback.broadcast_event) {
		err = GetLastError();
		goto error;
	}

	return 0;

error:
	CloseHandle(cond->fallback.signal_event);
error2:
	DeleteCriticalSection(&cond->fallback.waiters_count_lock);
	return -1;
}

static void __cond_fallback_destroy(SYS_UTL::CCond::LPLCOND_PROPERTY cond)
{
	if (!CloseHandle(cond->fallback.broadcast_event))
	{
		//abort();
		DBG_ERROR;
	}
	if (!CloseHandle(cond->fallback.signal_event))
	{
		//abort();
		DBG_ERROR;
	}
	DeleteCriticalSection(&cond->fallback.waiters_count_lock);
}

static void __cond_fallback_signal(SYS_UTL::CCond::LPLCOND_PROPERTY cond) 
{
	int have_waiters;

	/* Avoid race conditions. */
	EnterCriticalSection(&cond->fallback.waiters_count_lock);
	have_waiters = cond->fallback.waiters_count > 0;
	LeaveCriticalSection(&cond->fallback.waiters_count_lock);

	if (have_waiters)
		SetEvent(cond->fallback.signal_event);
}

static void __cond_fallback_broadcast(SYS_UTL::CCond::LPLCOND_PROPERTY cond) 
{
	int have_waiters;

	/* Avoid race conditions. */
	EnterCriticalSection(&cond->fallback.waiters_count_lock);
	have_waiters = cond->fallback.waiters_count > 0;
	LeaveCriticalSection(&cond->fallback.waiters_count_lock);

	if (have_waiters)
		SetEvent(cond->fallback.broadcast_event);
}

static int __cond_wait_helper(SYS_UTL::CCond::LPLCOND_PROPERTY cond, CRITICAL_SECTION& mutex,
	DWORD dwMilliseconds) 
{
	DWORD result;
	int last_waiter;
	HANDLE handles[2] = {
		cond->fallback.signal_event,
		cond->fallback.broadcast_event
	};

	/* Avoid race conditions. */
	EnterCriticalSection(&cond->fallback.waiters_count_lock);
	cond->fallback.waiters_count++;
	LeaveCriticalSection(&cond->fallback.waiters_count_lock);

	/* It's ok to release the <mutex> here since Win32 manual-reset events */
	/* maintain state when used with <SetEvent>. This avoids the "lost wakeup" */
	/* bug. */
	//uv_mutex_unlock(mutex);
	LeaveCriticalSection(&mutex);

	/* Wait for either event to become signaled due to <uv_cond_signal> being */
	/* called or <uv_cond_broadcast> being called. */
	result = WaitForMultipleObjects(2, handles, FALSE, dwMilliseconds);

	EnterCriticalSection(&cond->fallback.waiters_count_lock);
	cond->fallback.waiters_count--;
	last_waiter = result == WAIT_OBJECT_0 + 1
		&& cond->fallback.waiters_count == 0;
	LeaveCriticalSection(&cond->fallback.waiters_count_lock);

	/* Some thread called <pthread_cond_broadcast>. */
	if (last_waiter) {
		/* We're the last waiter to be notified or to stop waiting, so reset the */
		/* the manual-reset event. */
		ResetEvent(cond->fallback.broadcast_event);
	}

	/* Reacquire the <mutex>. */
	//uv_mutex_lock(mutex);
	EnterCriticalSection(&mutex);

	if (result == WAIT_OBJECT_0 || result == WAIT_OBJECT_0 + 1)
		return 0;

	if (result == WAIT_TIMEOUT)
		return -1;

	abort();
	return -2; /* Satisfy the compiler. */
}

static void __cond_fallback_wait(SYS_UTL::CCond::LPLCOND_PROPERTY cond, CRITICAL_SECTION& mutex) 
{
	if (__cond_wait_helper(cond, mutex, INFINITE))
		abort();
}

static int __cond_fallback_timedwait(SYS_UTL::CCond::LPLCOND_PROPERTY cond,
	CRITICAL_SECTION& mutex, DWORD timeout) 
{
	return __cond_wait_helper(cond, mutex, timeout);
}


namespace SYS_UTL
{

	CCond::CCond()
	{
		m_bInit = FALSE;
		m_iHas = 0;
		__Init();
	}

	CCond::~CCond()
	{
		__UnInit();
	}

	int CCond::__Init()
	{
		if (__IsInit())
		{
			return 0;
		}
		__sys_once_init();
		int err = 0;
		if (HAVE_CONDVAR_API())
		{
			DBG_INFO("HAVE_CONDVAR_API:%d", HAVE_CONDVAR_API());
			err = __cond_condvar_init(m_Cond.cond_var);
		}else
		{
			err = __cond_fallback_init(&m_Cond);
		}
		if (0 == err)
		{
			m_bInit = TRUE;
		}
		else{
			DBG_ERROR;
			abort();
		}
		return err;
	}

	void CCond::__UnInit()
	{
		if (!__IsInit())
		{
			return;
		}
		if (HAVE_CONDVAR_API())
		{
			__cond_condvar_destroy(m_Cond.cond_var);
		}else
		{
			__cond_fallback_destroy(&m_Cond);
		}
	}

	int CCond::Signal()
	{
		if (!__IsInit())
		{
			DBG_ERROR;
			return -1;
		}
		CAutoLock lck(&m_Mutex);
		m_iHas = 1;
		if (HAVE_CONDVAR_API())
			__cond_condvar_signal(m_Cond.cond_var);
		else
			__cond_fallback_signal(&m_Cond);
		return 0;
	}

	int CCond::Broadcast()
	{
		if (!__IsInit())
		{
			DBG_ERROR;
			return -1;
		}
		CAutoLock lck(&m_Mutex);
		m_iHas = 1;
		if (HAVE_CONDVAR_API())
			__cond_condvar_broadcast(m_Cond.cond_var);
		else
			__cond_fallback_broadcast(&m_Cond);
		return 0;

	}

	bool CCond::Wait()
	{
		if (!__IsInit())
		{
			DBG_ERROR;
			return false;
		}
		CAutoLock lck(&m_Mutex);
		m_iHas = 0;
		while (m_iHas <= 0)
		{//´æÔÚÐé¼Ù¼¤»î
			if (HAVE_CONDVAR_API())
			{
				__cond_condvar_wait(m_Cond.cond_var, m_Mutex.Get());
			}
			else
			{
				__cond_fallback_wait(&m_Cond, m_Mutex.Get());
			}
		}
		return true;
	}

	bool CCond::TryWait(DWORD dwTimeOut)
	{
		int err = 0;
		CAutoLock lck(&m_Mutex, SYS_UTL::LOCK_FLAG::lock_defer);
		if (!lck.Lock(dwTimeOut))
		{
			return false;
		}
		DWORD dwStart = GetTickCount();
		m_iHas = 0;
		while (m_iHas <= 0)
		{
			if (HAVE_CONDVAR_API())
			{
				err = __cond_condvar_timedwait(m_Cond.cond_var, m_Mutex.Get(), dwTimeOut);
			}
			else{
				err = __cond_fallback_timedwait(&m_Cond, m_Mutex.Get(), dwTimeOut);
			}
			if (abs((int)(GetTickCount()-dwStart)) >= dwTimeOut)
			{
				break;
			}
		}

		return (0 == err) && (m_iHas > 0);
	}

	BOOL CCond::__IsInit() const
	{
		return m_bInit;
	}

}