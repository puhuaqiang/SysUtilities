#include "../SysUtilities/stdafx.h"
#include "../include/AutoLock.h"
#include "../include/ThreadPool.h"

namespace SYS_UTL
{
	CThreadWrapper::CThreadWrapper()
	{
		m_uiIdleTimestamp = 0;
		m_iError = 0;
		ZeroMemory(&m_Task, sizeof(m_Task));
		ZeroMemory(&m_TaskCallBack, sizeof(m_TaskCallBack));
	}

	CThreadWrapper::~CThreadWrapper()
	{

	}

	int CThreadWrapper::Start(int iPacketSize, SYS_UTL::THREAD_POOL_PROCESS lpProcCallBack, void* lpUsr)
	{
		if (NULL == lpProcCallBack)
		{
			__UpdateError(-1);
			return -1;
		}
		m_Task.iBufferLen = 1024 * 8;
		m_Task.lpBuff = m_Task.buff;
		m_Task.iDataLen = 0;

		m_TaskCallBack.cb = lpProcCallBack;
		m_TaskCallBack.lpUsr = lpUsr;

		m_Thread.Init(iPacketSize, TRUE, TRUE);
		m_Thread.Start([this](BOOL& bRun, HANDLE hWait, void* context){
			int iTaskLen = 0;
			while (bRun)
			{
				//任务处理完成,进入空闲状态
				__UpdateIdleTime(GetTickCount());

				WaitForSingleObject(hWait, INFINITE);
				if (!bRun)
				{
					break;
				}

				if (NULL == m_Task.lpBuff)
				{
					__UpdateError(-1);
				}

				//正在处理任务.
				__UpdateIdleTime(0);

				do
				{
					if (NULL == m_Task.lpBuff)
					{
						__UpdateError(-1);
						break;
					}

					iTaskLen = 0;
					m_Thread.ReadData(m_Task.lpBuff, 1, &iTaskLen);
					if ((iTaskLen <= 0) || (iTaskLen > 0x0FFFFFFF))
					{
						break;
					}

					if (iTaskLen > m_Task.iBufferLen)
					{
						//重新分配缓冲区大小
						if (m_Task.iBufferLen > 1024 * 8)
						{
							if (NULL != m_Task.lpBuff)
							{
								delete[] m_Task.lpBuff;
								m_Task.lpBuff = NULL;
							}
						}
						m_Task.iBufferLen = PAD_SIZE(iTaskLen);
						m_Task.lpBuff = new BYTE[m_Task.iBufferLen];
						if (NULL == m_Task.lpBuff)
						{
							break;
						}
					}

					iTaskLen = m_Thread.ReadData(m_Task.lpBuff, m_Task.iBufferLen);
					if ((iTaskLen <= 0) || (iTaskLen > 0x0FFFFFFF))
					{
						__UpdateError(-2);
						break;
					}
					m_TaskCallBack.cb(m_Task.lpBuff, iTaskLen, m_TaskCallBack.lpUsr);

				} while (true);

			}

		});
		if (!m_Thread.IsStart())
		{
			return -1;
		}
		return 0;
	}

	void CThreadWrapper::Stop()
	{
		m_Thread.Cancel();
		m_Thread.Stop(100);
		m_Thread.UnInit();
		if (m_Task.iBufferLen > 1024 * 8)
		{
			if (NULL != m_Task.lpBuff)
			{
				delete[] m_Task.lpBuff;
				m_Task.lpBuff = NULL;
			}
		}
	}

	int CThreadWrapper::Task(void* lpTask, int iTaskDataLen)
	{
		if (NULL == lpTask)
		{
			return -1;
		}
		int err = m_Thread.WriteData(lpTask, iTaskDataLen);
		return err != 0 ? -1 : 0;
	}

	int CThreadWrapper::GetIdleTime()
	{
		CAutoLock lck(&m_lock);
		if (0 == m_uiIdleTimestamp)
		{
			return 0;
		}
		return abs((int)(GetTickCount() - m_uiIdleTimestamp));
	}

	bool CThreadWrapper::IsError()
	{
		CAutoLock lck(&m_lock);
		return 0 != m_iError;
	}

	bool CThreadWrapper::IsIdle()
	{
		CAutoLock lck(&m_lock);
		return m_uiIdleTimestamp > 0;
	}

	void CThreadWrapper::__UpdateIdleTime(unsigned int uiTimestamp)
	{
		CAutoLock lck(&m_lock);
		if (0 == uiTimestamp)
		{
			m_uiIdleTimestamp = 0;
			return;
		}
		if (m_uiIdleTimestamp == 0)
		{
			m_uiIdleTimestamp = uiTimestamp;
		}
	}

	void CThreadWrapper::__UpdateError(int err)
	{
		CAutoLock lck(&m_lock);
		m_iError = err;
	}

	CThreadPool::CThreadPool()
	{
		m_bInit = FALSE;
		m_bStartUp = FALSE;
		m_iMaxIdleTime = 30;
		m_iMaxThreadCnt = 0;
		m_iSingleTaskMaxLen = 0;
		ZeroMemory(&m_TaskCallBack, sizeof(m_TaskCallBack));
		m_Task.iBufferLen = 0;
		m_Task.iDataLen = 0;
		m_Task.lpBuff = NULL;
	}

	CThreadPool::~CThreadPool()
	{

	}

	int CThreadPool::Init(int iThreadCnt, int iTaskBufferLen, int iSingleTaskMaxLen, int iMaxIdleTime)
	{
		CAutoLock lck(&m_lock);
		if (0 >= iThreadCnt)
		{
			return -1;
		}
		if ((iTaskBufferLen < 0) || (iTaskBufferLen > 0x0FFFFFFF))
		{
			return -1;
		}
		if ((iSingleTaskMaxLen < 0) || (iSingleTaskMaxLen > iTaskBufferLen))
		{
			return -1;
		}
		m_iSingleTaskMaxLen = iSingleTaskMaxLen;
		m_iMaxThreadCnt = iThreadCnt;
		m_iMaxIdleTime = iMaxIdleTime;
		m_CtrlThread.Init(iTaskBufferLen, TRUE, TRUE);
		return 0;
	}

	void CThreadPool::UnInit()
	{
		CAutoLock lck(&m_lock);
		Stop();
		m_CtrlThread.UnInit();
	}

	int CThreadPool::Start(THREAD_POOL_PROCESS lpProcCallBack, void* lpUsr)
	{
		if (NULL == lpProcCallBack)
		{
			return -1;
		}
		CAutoLock lck(&m_lock);
		if (m_CtrlThread.IsStart())
		{
			return -2;
		}
		m_TaskCallBack.lpPorc = lpProcCallBack;
		m_TaskCallBack.lpUsr = lpUsr;

		m_Task.iBufferLen = 1024 * 8;
		m_Task.lpBuff = m_Task.buff;
		m_Task.iDataLen = 0;

		m_CtrlThread.Start(__CtrlThreadProc, this);
		if (!m_CtrlThread.IsStart())
		{
			return -2;
		}
		return 0;
	}

	int CThreadPool::Start(std::function<void(void* lpTask, int iTaskDataLen)> cb)
	{
		CAutoLock lck(&m_lock);
		if (m_CtrlThread.IsStart())
		{
			return -1;
		}

		m_cb = cb;

		m_Task.iBufferLen = 1024 * 8;
		m_Task.lpBuff = m_Task.buff;
		m_Task.iDataLen = 0;

		m_CtrlThread.Start(__CtrlThreadProc, this);
		if (!m_CtrlThread.IsStart())
		{
			return -2;
		}
		return 0;
	}

	void CThreadPool::Stop()
	{
		CAutoLock lck(&m_lock);
		for (auto& it : m_lstThreads)
		{
			it->Stop();
			delete it;
		}
		m_lstThreads.clear();

		m_CtrlThread.Cancel();
		m_CtrlThread.Stop(100);
	}

	int CThreadPool::Task(void* lpTask, int iTaskDataLen)
	{
		if (NULL == lpTask)
		{
			return -1;
		}
		int err = m_CtrlThread.WriteData(lpTask, iTaskDataLen);
		return 0 == err ? 0 : -2;
	}

	void CThreadPool::__CtrlThreadProc(BOOL& bRun, HANDLE hWait, void* context)
	{
		CThreadPool* lpObj = (CThreadPool*)context;
		if (NULL == lpObj)
		{
			return;
		}
		lpObj->__CtrlThreadProcLoop(bRun, hWait);
	}

	void CThreadPool::__CtrlThreadProcLoop(BOOL& bRun, HANDLE hWait)
	{
		int iTaskLen = 0;
		CThreadWrapper* lpProcThread;
		while (bRun)
		{
			WaitForSingleObject(hWait, 2000);
			if (!bRun)
			{
				break;
			}

			if (m_CtrlThread.IsData())
			{
				do
				{
					if (NULL == m_Task.lpBuff)
					{
						break;
					}

					if (!m_CtrlThread.IsData())
					{
						break;
					}

					lpProcThread = __GetThread();
					if (NULL == lpProcThread)
					{
						break;
					}

					iTaskLen = 0;
					m_CtrlThread.ReadData(m_Task.lpBuff, 1, &iTaskLen);
					if ((iTaskLen <= 0) || (iTaskLen > 0x0FFFFFFF))
					{
						break;
					}

					if (iTaskLen > m_Task.iBufferLen)
					{
						//重新分配缓冲区大小
						if (m_Task.iBufferLen > 1024 * 8)
						{
							if (NULL != m_Task.lpBuff)
							{
								delete[] m_Task.lpBuff;
								m_Task.lpBuff = NULL;
							}
						}
						m_Task.iBufferLen = PAD_SIZE(iTaskLen);
						m_Task.lpBuff = new BYTE[m_Task.iBufferLen];
						if (NULL == m_Task.lpBuff)
						{
							break;
						}
					}

					iTaskLen = m_CtrlThread.ReadData(m_Task.lpBuff, m_Task.iBufferLen);
					if ((iTaskLen <= 0) || (iTaskLen > 0x0FFFFFFF))
					{
						break;
					}
					lpProcThread->Task(m_Task.lpBuff, iTaskLen);

				} while (true);
			}

			__CheckProcThread();
		}
	}

	void CThreadPool::__ThreadCallBack(void* lpTask, int iTaskDataLen, void* lpUsr)
	{
		CThreadPool* lpObj = (CThreadPool*)lpUsr;
		if (NULL == lpObj)
		{
			return;
		}
		lpObj->__ThreadCallBackProc(lpTask, iTaskDataLen);
	}

	void CThreadPool::__ThreadCallBackProc(void* lpTask, int iTaskDataLen)
	{
		if (NULL != m_TaskCallBack.lpPorc)
		{
			m_TaskCallBack.lpPorc(lpTask, iTaskDataLen, m_TaskCallBack.lpUsr);
		}
		else{
			if (m_cb)
			{
				m_cb(lpTask, iTaskDataLen);
			}
		}
	}

	SYS_UTL::CThreadWrapper* CThreadPool::__GetThread()
	{
		CAutoLock lck(&m_lock);
		for (auto& it : m_lstThreads)
		{
			if (it->IsIdle())
			{
				return it;
			}
		}
		if (m_lstThreads.size() >= (unsigned int)m_iMaxThreadCnt)
		{
			//MSG_INFO("线程池,工作线程满负荷");
			return NULL;
		}
		CThreadWrapper* lpObj = new CThreadWrapper();
		if (NULL == lpObj)
		{
			return NULL;
		}
		int err = lpObj->Start(m_iSingleTaskMaxLen, __ThreadCallBack, this);
		if (0 != err)
		{
			return NULL;
		}
		m_lstThreads.emplace_back(lpObj);
		return lpObj;
	}

	void CThreadPool::__CheckProcThread()
	{
		CAutoLock lck(&m_lock);
		auto itBeg = m_lstThreads.begin();
		for (; itBeg != m_lstThreads.end();)
		{
			if ((*itBeg)->IsError() || ((*itBeg)->GetIdleTime() >= m_iMaxIdleTime))
			{
				/*MSG_INFO("空闲超时:%d 线程是否发生错误:%d",
				(*itBeg)->GetIdleTime() >= m_iMaxIdleTime,
				(*itBeg)->IsError());*/
				(*itBeg)->Stop();
				delete (*itBeg);
				itBeg = m_lstThreads.erase(itBeg);
				continue;
			}
			itBeg++;
		}
		//MSG_INFO("线程池.工作线程:%d", m_lstThreads.size());
	}
}
