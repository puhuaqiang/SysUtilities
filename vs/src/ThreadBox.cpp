#include "../SysUtilities/stdafx.h"
#include "../include/AutoLock.h"
#include "../include/ThreadBox.h"

typedef struct PacketHead_t
{
	DWORD dwFlags;
	DWORD dwLen;
}PacketHead_t;
#define PACKET_HEAD_FLAGS 0x64683030

namespace SYS_UTL
{
	CThreadBox::CThreadBox()
	{
		m_bInit = FALSE;

		m_hObject = 0;
		m_hWait = NULL;
		m_bDone = TRUE;
		m_bRun = FALSE;
		m_dwID = 0;
		m_pProcess = NULL;
		m_ProcessContext = NULL;

		m_pBuffer = NULL;
		m_iBufferLen = 0;
		m_iWritePos = 0;
		m_iReadPos = 0;
		m_iDataLen = 0;

		m_bPacket = TRUE;
		m_bCreateEvent = TRUE;
		ZeroMemory(m_szName, sizeof(m_szName));
	}

	CThreadBox::~CThreadBox()
	{

	}

	struct _CONTEXT_THREAD_PAR
	{
		std::function<void(void*)> cb;
		std::function<void(void* lpData, int len, void* pParam)> cb2;
		void* par;
		int len;
		BYTE buff[1024];
	};

	DWORD WINAPI CallThreadFn(void* par)
	{
		_CONTEXT_THREAD_PAR* p = (_CONTEXT_THREAD_PAR*)par;
		if (NULL == p){
			return -1;
		}
		if (p->len > 0)
		{
			if (p->cb2)
			{
				p->cb2(p->buff, p->len, p->par);
			}
		}
		else{
			if (p->cb)
			{
				p->cb(p->par);
			}
		}
		delete p;
		return 0;
	}

	bool CThreadBox::RunEx(std::function<void(void*)> cb, void* pParam)
	{
		_CONTEXT_THREAD_PAR* p = new _CONTEXT_THREAD_PAR();
		if (NULL == p){
			return false;
		}
		p->cb = cb;
		p->par = pParam;
		p->len = 0;
		DWORD dwThreadId = 0;
		HANDLE hThread = CreateThread(0, 0, CallThreadFn, p, 0, &dwThreadId);
		if (hThread) {
			CloseHandle(hThread);
			return true;
		}
		return false;
	}

	bool CThreadBox::RunEx2(std::function<void(void* lpData, int len, void* pParam)> cb, void* lpData, int len, void* pParam)
	{
		if (len <= 0 || (lpData == NULL))
		{
			return false;
		}
		if (len > 1024)
		{
			return false;
		}
		_CONTEXT_THREAD_PAR* p = new _CONTEXT_THREAD_PAR();
		if (NULL == p){
			return false;
		}
		p->cb2 = cb;
		p->par = pParam;
		p->len = len;
		memcpy(p->buff, lpData, len);
		DWORD dwThreadId = 0;
		HANDLE hThread = CreateThread(0, 0, CallThreadFn, p, 0, &dwThreadId);
		if (hThread) {
			CloseHandle(hThread);
			return true;
		}
		return false;
	}

	bool CThreadBox::Run(DWORD(WINAPI* cb)(void*), void* pParam)
	{
		DWORD dwThreadId = 0;
		HANDLE hThread = CreateThread(0, 0, cb, pParam, 0, &dwThreadId);
		if (hThread) {
			CloseHandle(hThread);
			return true;
		}
		return false;
	}

	int	CThreadBox::Init(int iPacketSize /*= 1024*/, BOOL bPacket /*= TRUE*/, BOOL bCreateEvent /*= TRUE*/)
	{
		CAutoLock Lock(&m_mutexPacket);

		m_iBufferLen = PAD_SIZE(iPacketSize);
		if (m_iBufferLen > 0)
		{
			m_pBuffer = (BYTE*)malloc(m_iBufferLen);
		}

		m_iWritePos = 0;
		m_iReadPos = 0;
		m_iDataLen = 0;

		m_bPacket = bPacket;
		m_bCreateEvent = bCreateEvent;
		m_bInit = TRUE;

		return 0;
	}

	int CThreadBox::Init(int iPacketSize /*= 1024*/, BOOL bPacket, BOOL bCreateEvent, const char* lpName)
	{
		if (NULL != lpName)
		{
			_snprintf_s(m_szName, _TRUNCATE, "%s", lpName);
		}
		return Init(iPacketSize, bPacket, bCreateEvent);
	}

	void CThreadBox::UnInit()
	{
		CAutoLock Lock(&m_mutexPacket);

		m_bInit = FALSE;

		if (NULL != m_pBuffer)
		{
			free(m_pBuffer);
			m_pBuffer = NULL;
			m_iBufferLen = 0;
		}
		m_iWritePos = 0;
		m_iReadPos = 0;
		m_iDataLen = 0;

		if (m_hWait != NULL)
		{
			CloseHandle(m_hWait);
			m_hWait = NULL;
		}
	}

	BOOL CThreadBox::IsStart()
	{
		return (!m_bDone);
	}

	BOOL CThreadBox::IsData()
	{
		CAutoLock Lock(&m_mutexPacket);
		return (m_iDataLen > 0);
	}

	void CThreadBox::ResetData()
	{
		CAutoLock Lock(&m_mutexPacket);
		m_iWritePos = 0;
		m_iReadPos = 0;
		m_iDataLen = 0;
	}

	int	CThreadBox::Start(SYS_UTL::BOX_THREAD_PROCESS lpProcess, void* pUserContext)
	{
		if (m_bDone)
		{
			m_pProcess = lpProcess;
			m_ProcessContext = pUserContext;

			if (m_hObject != NULL)
			{
				CloseHandle(m_hObject);
				m_hObject = NULL;
			}

			m_bRun = TRUE;
			m_bDone = FALSE;
			if ((m_hWait == NULL) && m_bCreateEvent)
			{
				m_hWait = ::CreateEvent(NULL, FALSE, FALSE, NULL);
			}
			m_hObject = CreateThread(NULL, 0, Process, (LPVOID)this, 0, &m_dwID);
			if (m_hObject == NULL)
			{
				m_bRun = FALSE;
				m_bDone = TRUE;
				return -1;
			}
		}

		return 0;
	}

	int CThreadBox::Start(std::function<void(BOOL& bRun, HANDLE hWait, void* context)> cb) 
	{
		if (!m_bDone) 
		{
			return -1;
		}
		m_cb = cb;

		if (m_hObject != NULL)
		{
			CloseHandle(m_hObject);
			m_hObject = NULL;
		}

		m_bRun = TRUE;
		m_bDone = FALSE;
		if ((m_hWait == NULL) && m_bCreateEvent) 
		{
			m_hWait = CreateEvent(NULL, FALSE, FALSE, NULL);
		}
		m_hObject = CreateThread(NULL, 0, Process, (LPVOID)this, 0, &m_dwID);
		if (m_hObject == NULL)
		{
			m_bRun = FALSE;
			m_bDone = TRUE;
			return -2;
		}
		return 0;
	}

	int	CThreadBox::Stop(DWORD dwTimeOut)
	{
		if (!m_bDone)
		{
			DWORD dwRet;
			BOOL bExit;

			m_bRun = FALSE;
			if (m_hWait != NULL)
			{
				::SetEvent(m_hWait);
			}
			Sleep(10);
			dwRet = WaitForSingleObject(m_hObject, dwTimeOut);
			if (dwRet != WAIT_OBJECT_0)
			{
				bExit = GetExitCodeThread(m_hObject, &dwRet);
				if (bExit)
				{
					TerminateThread(m_hObject, dwRet);
				}
			}
			m_bDone = TRUE;
		}
		if (m_hObject != NULL)
		{
			CloseHandle(m_hObject);
			m_hObject = NULL;
		}
		if (m_hWait != NULL && m_bCreateEvent)
		{
			CloseHandle(m_hWait);
			m_hWait = NULL;
		}

		return 0;
	}

	DWORD WINAPI CThreadBox::Process(void* lpThis)
	{
		//static int thread_num = 0;
		CThreadBox* pBox = (CThreadBox*)lpThis;

		//thread_num += 1;
		if (NULL != pBox->m_pProcess)
		{
			pBox->m_pProcess(pBox->m_bRun, pBox->m_hWait, pBox->m_ProcessContext);
		}
		else{
			pBox->m_cb(pBox->m_bRun, pBox->m_hWait, pBox->m_ProcessContext);
		}
		pBox->m_bDone = TRUE;

		//thread_num -= 1;

		return 0;
	}

	int	CThreadBox::Cancel()
	{
		if (m_bDone)
		{
			return -1;
		}

		m_bRun = FALSE;
		if (m_hWait != NULL)
		{
			::SetEvent(m_hWait);
		}

		return 0;
	}

	int CThreadBox::OpenEvent()
	{
		if (m_hWait == NULL)
		{
			m_hWait = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		}
		return 0;
	}

	void CThreadBox::CloseEvent()
	{
		if (m_hWait != NULL)
		{
			::CloseHandle(m_hWait);
		}
	}

	void CThreadBox::SetEvent()
	{
		if (m_hWait != NULL)
		{
			::SetEvent(m_hWait);
		}
	}

	void CThreadBox::ResetEvent()
	{
		if (m_hWait != NULL)
		{
			::ResetEvent(m_hWait);
		}
	}

	HANDLE CThreadBox::GetEventHandle()
	{
		return m_hWait;
	}

	int CThreadBox::WriteData(void* pData, int iDataLen, BOOL bEvent)
	{
		void* buf[1];
		int	  len[1];

		buf[0] = pData;
		len[0] = iDataLen;

		return WriteDataEx(buf, len, 1, bEvent);
	}

	int CThreadBox::WriteDataEx(void* pData[], int iDataLen[], int iNum, BOOL bEvent)
	{
		CAutoLock Lock(&m_mutexPacket);

		int i, iSize, iDataTotalLen, iLen;
		BYTE* head = NULL;

		if (!m_bInit)
		{
			return -1;
		}

		if (m_pBuffer == NULL)
		{
			return -2;
		}

		iDataTotalLen = 0;
		for (i = 0; i < iNum; i++)
		{
			iDataTotalLen += iDataLen[i];
		}

		if (m_bPacket)
		{
			PacketHead_t pkt;

			iLen = iDataTotalLen + sizeof(PacketHead_t);
			iSize = m_iBufferLen - m_iDataLen;
			if (iSize < iLen)
			{
				return -3;
			}

			memset(&pkt, 0, sizeof(PacketHead_t));
			pkt.dwFlags = PACKET_HEAD_FLAGS;
			pkt.dwLen = iDataTotalLen;

			head = (BYTE*)&pkt;
			iLen = sizeof(PacketHead_t);
			iSize = m_iBufferLen - m_iWritePos;
			if (iSize < iLen)
			{
				memcpy(m_pBuffer + m_iWritePos, head, iSize);
				memcpy(m_pBuffer, head + iSize, iLen - iSize);
			}
			else
			{
				memcpy(m_pBuffer + m_iWritePos, head, iLen);
			}
			m_iDataLen += iLen;
			m_iWritePos += iLen;
			if (m_iWritePos >= m_iBufferLen)
			{
				m_iWritePos -= m_iBufferLen;
			}
		}
		else
		{
			iSize = m_iBufferLen - m_iDataLen;
			if (iSize < iDataTotalLen)
			{
				return -3;
			}
		}

		for (i = 0; i<iNum; i++)
		{
			head = (BYTE*)pData[i];
			iLen = iDataLen[i];
			if (iLen > 0)
			{
				iSize = m_iBufferLen - m_iWritePos;
				if (iSize < iLen)
				{
					memcpy(m_pBuffer + m_iWritePos, head, iSize);
					memcpy(m_pBuffer, head + iSize, iLen - iSize);
				}
				else
				{
					memcpy(m_pBuffer + m_iWritePos, head, iLen);
				}
				m_iDataLen += iLen;
				m_iWritePos += iLen;
				if (m_iWritePos >= m_iBufferLen)
				{
					m_iWritePos -= m_iBufferLen;
				}
			}
		}

		if (bEvent && m_hWait != NULL)
		{
			::SetEvent(m_hWait);
		}

		return 0;
	}

	int CThreadBox::ReadData(void* pData, int iDataLen, int* lpRequestSize, BOOL bForceGet)
	{
		CAutoLock Lock(&m_mutexPacket);

		int iLen, data_len;
		BYTE* buf;
		PacketHead_t pkt;

		if (!m_bInit)
		{
			return -1;
		}

		if (m_iDataLen <= 0)
		{
			return -2;
		}

		if (m_bPacket)
		{
			buf = (BYTE*)&pkt;
			data_len = sizeof(PacketHead_t);
			iLen = m_iBufferLen - m_iReadPos;
			if (data_len <= iLen)
			{
				memcpy(buf, m_pBuffer + m_iReadPos, data_len);
			}
			else
			{
				memcpy(buf, m_pBuffer + m_iReadPos, iLen);
				memcpy(buf + iLen, m_pBuffer, data_len - iLen);
			}

			if (pkt.dwFlags != PACKET_HEAD_FLAGS)
			{
				m_iWritePos = 0;
				m_iReadPos = 0;
				m_iDataLen = 0;
				return -3;
			}

			if (iDataLen < (int)pkt.dwLen)
			{
				if (lpRequestSize != NULL)
				{
					*lpRequestSize = (int)pkt.dwLen;
				}
				return 0;
			}

			m_iDataLen -= data_len;
			m_iReadPos += data_len;
			if (m_iReadPos >= m_iBufferLen)
			{
				m_iReadPos -= m_iBufferLen;
			}

			data_len = (int)pkt.dwLen;
		}
		else
		{
			if (m_iDataLen < iDataLen)
			{
				if (!bForceGet)
				{
					return -4;
				}
			}
			data_len = iDataLen;
		}

		if (data_len > 0)
		{
			buf = (BYTE*)pData;
			iLen = m_iBufferLen - m_iReadPos;
			if (data_len <= iLen)
			{
				memcpy(buf, m_pBuffer + m_iReadPos, data_len);
			}
			else
			{
				memcpy(buf, m_pBuffer + m_iReadPos, iLen);
				memcpy(buf + iLen, m_pBuffer, data_len - iLen);
			}
			m_iDataLen -= data_len;
			m_iReadPos += data_len;
			if (m_iReadPos >= m_iBufferLen)
			{
				m_iReadPos -= m_iBufferLen;
			}
		}

		return data_len;
	}
	DWORD CThreadBox::GetCurrentThreadId()
	{
		return GetThreadId(m_hObject);
	}

	const char* CThreadBox::GetThreadName()
	{
		return m_szName;
	}

	CTaskThread::CTaskThread()
	{
		m_TaskThread.Init(0, TRUE, TRUE);
		m_TaskThread.Start([this](BOOL& bRun, HANDLE hWait, void* context){
			std::queue<std::function<void()>> tasks_;
			while (bRun)
			{
				WaitForSingleObject(hWait, 5000 /*INFINITE*/);
				if (!bRun)
				{
					break;
				}
			{
				CAutoLock lck(&m_lckTask);
				if (!m_Tasks.empty())
				{
					do
					{
						tasks_.emplace(m_Tasks.front());
						m_Tasks.pop();
					} while (!m_Tasks.empty());
				}
			}

			if (tasks_.empty())
			{
				continue;
			}
			m_TaskProcing.store(true);
			do
			{
				if (tasks_.front())
				{
					tasks_.front()();
				}
				tasks_.pop();
			} while (!tasks_.empty());
			m_TaskProcing.store(false);
			}
		});

		m_IdleThread.Init(0, TRUE, TRUE);
		m_IdleThread.Start([this](BOOL& bRun, HANDLE hWait, void* context){
			std::queue<std::function<void()>> tasks_;
			while (bRun)
			{
				WaitForSingleObject(hWait, 5000);
				if (!bRun)
				{
					break;
				}

			{
				CAutoLock lck(&m_lckIdleTask);
				if (!m_idleTasks.empty())
				{
					do
					{
						tasks_.emplace(m_idleTasks.front());
						m_idleTasks.pop();
					} while (!m_idleTasks.empty());
				}
			}

			if (tasks_.empty())
			{
				continue;
			}

			if (m_TaskProcing.load())
			{
				continue;
			}

			do
			{
				if (tasks_.front())
				{
					tasks_.front()();
				}
				tasks_.pop();
				if (m_TaskProcing.load())
				{
					break;
				}
			} while (!tasks_.empty());
			}
		});
	}


	CTaskThread::~CTaskThread()
	{
		m_IdleThread.Cancel();
		m_IdleThread.Stop(1000);
		m_IdleThread.UnInit();

		m_TaskThread.Cancel();
		m_TaskThread.Stop(1000);
		m_TaskThread.UnInit();
	}

}