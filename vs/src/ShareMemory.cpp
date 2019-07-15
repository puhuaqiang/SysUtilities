#include "../SysUtilities/stdafx.h"
#include "../include/ShareMemory.h"
#include "InDef.h"

namespace SYS_UTL
{
	//共享数据段头部
	struct  DataHeader_t {
		DWORD dwMaxSize;//数据段总长度	
		DWORD dwUsedSize;//已经使用的空间大小
		DWORD dwWriteOffset;//数据写入位置相对首地址的偏移地址	
		DWORD dwReadOffset;//当前读取位置相对首地址的偏移地址
	};

	CShareMemory::CShareMemory()
		: m_bInit(FALSE)
		, m_hFileMapping(NULL)
		, m_nType(CT_WRITER)
		, m_lpBasePointer(NULL)
	{
		ZeroMemory(&m_stCallBack, sizeof(m_stCallBack));
	}

	CShareMemory::~CShareMemory()
	{
		UnInit();
	}

	int	CShareMemory::Init(CommunicatorType nType, PSHARE_MEMORY_PROPERTY lpProperty, DWORD dwLength)
	{
		//指向共享内存段的首地址
		DataHeader_t* lpDataHeader = NULL;
		BOOL bNeedInitHeader = TRUE;
		SECURITY_ATTRIBUTES attr;
		PSECURITY_DESCRIPTOR lpSec = { NULL };
		int	nErrCode = 0;

		if (__IsInit())
		{
			return nErrCode;
		}

		if (dwLength <= sizeof(DataHeader_t) || (lpProperty == NULL))
		{
			DBG_E;
			return -1;
		}
		if (NULL == lpProperty->szShareSegName ||
			NULL == lpProperty->szChangeEventName ||
			NULL == lpProperty->szMutexName ||
			strlen(lpProperty->szShareSegName) <= 0 ||
			strlen(lpProperty->szChangeEventName) <= 0 ||
			strlen(lpProperty->szMutexName) <= 0)
		{
			DBG_E;
			return -2;
		}

		//设置共享属性，使共享数据段能被其他进程访问
		lpSec = (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_FIXED, SECURITY_DESCRIPTOR_MIN_LENGTH);
		if (NULL == lpSec)
		{
			DBG_E;
			return -3;
		}
		if (!InitializeSecurityDescriptor(lpSec, SECURITY_DESCRIPTOR_REVISION))
		{
			DBG_E;
			nErrCode = -4;
			goto EXIT;
		}
		if (!SetSecurityDescriptorDacl(lpSec, TRUE, NULL, TRUE))
		{
			DBG_E;
			nErrCode = -5;
			goto EXIT;
		}
		attr.bInheritHandle = FALSE;
		attr.lpSecurityDescriptor = lpSec;
		attr.nLength = sizeof(SECURITY_ATTRIBUTES);

		m_nType = nType;

		//创建(或打开)共享数据段
		DWORD dwSize = dwLength + sizeof(DataHeader_t);
		dwSize = PAD_SIZE(dwSize);
		m_hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, &attr, PAGE_READWRITE, 0, dwSize, lpProperty->szShareSegName);
		if (NULL == m_hFileMapping) 
		{
			DBG_E;
			nErrCode = -6;
			goto EXIT;
		}
		if (ERROR_ALREADY_EXISTS == GetLastError())
		{//缓存区已经存在，则无需再初始化缓存区
			bNeedInitHeader = FALSE;
		}
		m_lpBasePointer = (BYTE*)MapViewOfFile(m_hFileMapping, FILE_MAP_WRITE, 0, 0, 0);
		if (NULL == m_lpBasePointer)
		{
			DBG_E;
			nErrCode = -7;
			goto EXIT;
		}
		//将共享区域信息写入头部
		if (bNeedInitHeader) 
		{
			lpDataHeader = (DataHeader_t*)m_lpBasePointer;
			ZeroMemory(lpDataHeader, sizeof(DataHeader_t));
			lpDataHeader->dwMaxSize = dwLength;
			lpDataHeader->dwUsedSize = 0;
			lpDataHeader->dwWriteOffset = 0;
			lpDataHeader->dwReadOffset = 0;
		}
		//创建(或打开)数据写入通知
		if (!m_WriteEvent.Init(&attr, FALSE, FALSE, (const char*)lpProperty->szChangeEventName))
		{
			DBG_E;
			nErrCode = -8;
			goto EXIT;
		}
		//创建(或打开)数据段访问互斥锁
		if (!m_Mutex.Init(&attr, FALSE, lpProperty->szMutexName))
		{
			DBG_E;
			nErrCode = -9;
			goto EXIT;
		}
		//如果为读取模式，创建数据读取线程
		if (CT_READER == m_nType) 
		{
			m_read_thr.Init(0, TRUE, TRUE);
			m_read_thr.Start(ReadDataThread, this);
		}

		m_bInit = TRUE;
	EXIT:
		LocalFree(lpSec);
		lpSec = NULL;
		UnInit();
		return nErrCode;
	}

	void CShareMemory::UnInit()
	{
		if (CT_READER == m_nType) 
		{
			m_read_thr.Cancel();
			m_read_thr.Stop();
			m_read_thr.UnInit();
		}

		if (NULL != m_lpBasePointer) 
		{
			UnmapViewOfFile(m_lpBasePointer);
			m_lpBasePointer = NULL;
		}

		if (NULL != m_hFileMapping) 
		{
			CloseHandle(m_hFileMapping);
			m_hFileMapping = NULL;
		}

		m_WriteEvent.UnInit();
		m_Mutex.UnInit();
	}

	int CShareMemory::WriteData(CommunicatorPacket_t* pData)
	{
		DWORD dwWaitResult = 0;
		DataHeader_t*	lpDataHeader = NULL;
		DWORD dwDataLen = 0;
		int Err = 0;

		if (!__IsInit())
		{
			DBG_E;
			return -1;
		}
		if (CT_WRITER != m_nType)
		{
			DBG_E;
			return -2;
		}
		if (NULL == pData)
		{
			DBG_E;
			return -3;
		}
		if (PROC_FLAG != pData->dwFlag)
		{
			DBG_E;
			return -4;
		}
		//锁定共享缓存
		CAutoMutex lck(&m_Mutex, SYS_UTL::LOCK_FLAG::lock_defer);
		if (!m_Mutex.TryWait(2000))
		{
			DBG_E;
			return -5;
		}

		//读取共享数据段头部，获取写入位置等信息
		lpDataHeader = (DataHeader_t*)m_lpBasePointer;

		dwDataLen = sizeof(CommunicatorPacket_t) + pData->iLen;
		if (dwDataLen > lpDataHeader->dwMaxSize) 
		{
			Err = -6;//数据比总空间还大
			DBG_E;
			goto EXIT;
		}

		//判断从<当前位置>至<缓冲区尾>写入数据，空间是否满足	
		DWORD dwEmptyBuffSize = lpDataHeader->dwMaxSize - lpDataHeader->dwWriteOffset;
		if (dwEmptyBuffSize < dwDataLen) 
		{
			//空间不足，则判断能否从缓冲头再次写入数据
			if (lpDataHeader->dwReadOffset < dwDataLen) {
				Err = -7;//剩余空间不满足数据长度要求
				goto EXIT;
			}
			else {//再次从头开始写
				lpDataHeader->dwWriteOffset = 0;
			}
		}

		BYTE* ptr = m_lpBasePointer + (sizeof(DataHeader_t) + lpDataHeader->dwWriteOffset);
		memcpy(ptr, pData, dwDataLen);
		lpDataHeader->dwWriteOffset += dwDataLen;
		lpDataHeader->dwUsedSize += dwDataLen;

		Err = 0;
	EXIT:
		if (0 == Err)
		{
			m_WriteEvent.Set();
		}
		return Err;
	}

	void CShareMemory::RegisterDataCallback(std::function<void(CommunicatorPacket_t* lpPacket)> pCallback)
	{
		CAutoMutex lck(&m_Mutex);
		m_cb = pCallback;
	}

	void CShareMemory::RegisterDataCallback(void(*pCallback)(CommunicatorPacket_t* lpPacket, void* pUsrPar), void* pUsrPar)
	{
		CAutoMutex lck(&m_Mutex);
		m_stCallBack.lpFun = pCallback;
		m_stCallBack.lpUsr = pUsrPar;
	}

	void CShareMemory::ResetData()
	{
		if (!__IsInit())
		{
			DBG_E;
			return;
		}
		if (CT_READER != m_nType)
		{
			return;
		}
		CAutoMutex lck(&m_Mutex, SYS_UTL::LOCK_FLAG::lock_defer);
		if (!m_Mutex.TryWait(5000))
		{
			DBG_E;
			return;
		}
		DataHeader_t* lpDataHeader = (DataHeader_t*)m_lpBasePointer;
		lpDataHeader->dwReadOffset = 0;
		lpDataHeader->dwUsedSize = 0;
		lpDataHeader->dwWriteOffset = 0;
	}

	void CShareMemory::ReadData(BOOL& bRun, HANDLE hWait)
	{
		DWORD dwWait = 0;
		DataHeader_t*	lpDataHeader = NULL;
		lpDataHeader = (DataHeader_t*)m_lpBasePointer;
		CommunicatorPacket_t* lpPacket;
		HANDLE ahObject[2] = { hWait, m_Mutex.Get() };

		while (bRun) 
		{
			dwWait = WaitForMultipleObjects(2, ahObject, FALSE, 1000);
			if ((WAIT_OBJECT_0 + 1) != dwWait)
			{
				continue;
			}
			if (lpDataHeader->dwUsedSize <= 0) 
			{
				ReleaseMutex(ahObject[1]);
				break;//数据已全部读完
			}

			if ((lpDataHeader->dwMaxSize - lpDataHeader->dwReadOffset) < sizeof(CommunicatorPacket_t))
			{
				lpDataHeader->dwReadOffset = 0;
			}
			lpPacket = (CommunicatorPacket_t*)(m_lpBasePointer + (sizeof(DataHeader_t) + lpDataHeader->dwReadOffset));
			if (PROC_FLAG != lpPacket->dwFlag)
			{
				lpDataHeader->dwReadOffset = 0;
			}
			lpPacket = (CommunicatorPacket_t*)(m_lpBasePointer + (sizeof(DataHeader_t)+lpDataHeader->dwReadOffset));
			if (PROC_FLAG != lpPacket->dwFlag) 
			{
				ReleaseMutex(ahObject[1]);
				break;
			}
			__try
			{
				//避免数据无法写入，在数据输出过程中可以继续写入数据
				ReleaseMutex(ahObject[1]);
				//将数据输出给用户
				if (NULL != m_stCallBack.lpFun) {
					m_stCallBack.lpFun(lpPacket, m_stCallBack.lpUsr);
				}
				else{
					if (m_cb)
					{
						m_cb(lpPacket);
					}
				}
				//标志复位，防止重复输出，若超时而未被修改，则数据将被重复输出
				dwWait = WaitForMultipleObjects(2, ahObject, FALSE, 5000);
				if ((WAIT_OBJECT_0 + 1) == dwWait) 
				{
					lpDataHeader->dwUsedSize -= sizeof(CommunicatorPacket_t) + lpPacket->iLen;
					lpDataHeader->dwReadOffset += sizeof(CommunicatorPacket_t) + lpPacket->iLen;
					lpPacket->dwFlag = 0;
					ReleaseMutex(ahObject[1]);
				}
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				ReleaseMutex(ahObject[1]);
				break;
			}
		}
	}

	void CShareMemory::ReadDataThread(BOOL& bRun, HANDLE hWait, void* pUsrPar)
	{
		CShareMemory* self = (CShareMemory*)pUsrPar;
		HANDLE ahEvent[2] = { hWait, self->m_WriteEvent.Get() };
		DWORD dwWait = 0;
		while (bRun) 
		{
			//等待线程退出通知及数据写入通知
			dwWait = WaitForMultipleObjects(2, ahEvent, FALSE, INFINITE);
			if (!bRun)
			{
				break;
			}
			if ((WAIT_OBJECT_0 + 1) == dwWait)
			{
				self->ReadData(bRun, hWait);
			}
		}
	}

	struct tagPacketHead
	{
		int iDataLen;
	};
	CFixShareMemory::CFixShareMemory()
	{
		m_bInit = FALSE;
		m_hFileMapping = NULL;
		m_lpBasePointer = NULL;
		m_nType = CT_WRITER;
		m_iMaxPerPacketSize = 0;
		m_iMaxPacketCount = 0;
	}

	CFixShareMemory::~CFixShareMemory()
	{
		UnInit();
	}

	int CFixShareMemory::Init(CommunicatorType nType, const char* lpszShareMemoryName, int iMaxPerPacketSize, int iMaxPacketCount)
	{
		CAutoMutex lck(&m_Mutex);
		SECURITY_ATTRIBUTES	attr;
		PSECURITY_DESCRIPTOR lpSec = { NULL };
		int	nErrCode = 0;
		DWORD dwPacketSize = 0;

		if (NULL == lpszShareMemoryName)
		{
			DBG_E;
			return 0;
		}

		if (__IsInit())
		{
			return 0;
		}
		if ((iMaxPacketCount <= 0) || (iMaxPerPacketSize <= 0))
		{
			DBG_E;
			return -2;
		}
		if (iMaxPerPacketSize*iMaxPacketCount > 0x0FFFFFFF)
		{
			DBG_E;
			return -3;
		}
	
		//设置共享属性，使共享数据段能被其他进程访问
		lpSec = (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_FIXED, SECURITY_DESCRIPTOR_MIN_LENGTH);
		if (NULL == lpSec)
		{
			DBG_E;
			return -4;
		}
		if (!InitializeSecurityDescriptor(lpSec, SECURITY_DESCRIPTOR_REVISION))
		{
			DBG_E;
			nErrCode = -5;
			goto EXIT;
		}
		if (!SetSecurityDescriptorDacl(lpSec, TRUE, NULL, TRUE))
		{
			DBG_E;
			nErrCode = -6;
			goto EXIT;
		}
		attr.bInheritHandle = FALSE;
		attr.lpSecurityDescriptor = lpSec;
		attr.nLength = sizeof(SECURITY_ATTRIBUTES);

		//创建(或打开)共享数据段
		dwPacketSize = iMaxPerPacketSize*iMaxPacketCount + sizeof(tagPacketHead)*iMaxPacketCount;
		dwPacketSize = PAD_SIZE(dwPacketSize);
		m_hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, &attr, PAGE_READWRITE, 0, dwPacketSize, lpszShareMemoryName);
		if (NULL == m_hFileMapping)
		{
			DBG_E;
			nErrCode = -7;
			goto EXIT;
		}
		if (ERROR_ALREADY_EXISTS == GetLastError())
		{
			//缓存区已经存在，则无需再初始化缓存区				
		}
		m_lpBasePointer = (BYTE*)MapViewOfFile(m_hFileMapping, /*FILE_MAP_WRITE*/FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (NULL == m_lpBasePointer)
		{
			DBG_E;
			nErrCode = -8;
			goto EXIT;
		}

		m_nType = nType;
		m_iMaxPacketCount = iMaxPacketCount;
		m_iMaxPerPacketSize = iMaxPerPacketSize;
		m_bInit = TRUE;
	EXIT:
		LocalFree(lpSec);
		lpSec = NULL;

		return nErrCode;
	}

	void CFixShareMemory::UnInit()
	{
		CAutoMutex lck(&m_Mutex,SYS_UTL::LOCK_FLAG::lock_defer);
		if (!lck.TryWait(1000))
		{
			DBG_E;
			return;
		}
		m_bInit = FALSE;
		if (NULL != m_lpBasePointer)
		{
			UnmapViewOfFile(m_lpBasePointer);
			m_lpBasePointer = NULL;
		}
		if (NULL != m_hFileMapping)
		{
			CloseHandle(m_hFileMapping);
			m_hFileMapping = NULL;
		}
	}

	int CFixShareMemory::Write(int iPacketIndex, BYTE* pDataBuff, int iDataLen, DWORD dwTimeOut)
	{
		if (iPacketIndex < 0 || iPacketIndex >= m_iMaxPacketCount)
		{
			DBG_E;
			return -1;
		}
		if (NULL == pDataBuff || iDataLen <= 0 || iDataLen > m_iMaxPerPacketSize)
		{
			DBG_E;
			return -2;
		}
		if (!__IsInit())
		{
			DBG_E;
			return -3;
		}
		if (CT_WRITER != m_nType)
		{
			DBG_E;
			return -4;
		}

		CAutoMutex lck(&m_Mutex, SYS_UTL::LOCK_FLAG::lock_defer);
		if (!lck.TryWait(dwTimeOut))
		{
			DBG_E;
			return -1;
		}

		int len1 = m_iMaxPerPacketSize*iPacketIndex;
		int len2 = iPacketIndex*sizeof(tagPacketHead);
		tagPacketHead* pHead = (tagPacketHead*)(m_lpBasePointer + len1 + len2);
		pHead->iDataLen = iDataLen;
		BYTE* ptr = (BYTE*)(m_lpBasePointer + len1 + len2 + sizeof(tagPacketHead));
		memcpy(ptr, pDataBuff, iDataLen);

		return 0;
	}

	int CFixShareMemory::Read(int iPacketIndex, BYTE* pDataBuff, int iBuffLen, int* pDataLen, DWORD dwTimeOut)
	{
		if (iPacketIndex < 0 || iPacketIndex >= m_iMaxPacketCount)
		{
			DBG_E;
			return -1;
		}
		if (NULL == pDataBuff || iBuffLen <= 0 || iBuffLen < m_iMaxPerPacketSize)
		{
			DBG_E;
			return -1;
		}
		if (!__IsInit())
		{
			DBG_E;
			return -3;
		}
		if (CT_READER != m_nType)
		{
			DBG_E;
			return -4;
		}

		CAutoMutex lck(&m_Mutex, SYS_UTL::LOCK_FLAG::lock_defer);
		if (!lck.TryWait(dwTimeOut))
		{
			DBG_E;
			return -1;
		}

		int len1 = m_iMaxPerPacketSize*iPacketIndex;
		int len2 = iPacketIndex*sizeof(tagPacketHead);
		tagPacketHead* pHead = (tagPacketHead*)(m_lpBasePointer + len1 + len2);
		int iDataLen = pHead->iDataLen;
		BYTE* ptr = (BYTE*)(m_lpBasePointer + +len1 + len2 + sizeof(tagPacketHead));
		memcpy(pDataBuff, ptr, iDataLen);

		if (NULL != pDataLen)
		{
			*pDataLen = iDataLen;
		}

		return 0;
	}

	int CFixShareMemory::Write(int iPacketIndex, BYTE* pDataBuff, int iDataLen)
	{
		return Write(iPacketIndex, pDataBuff, iDataLen, INFINITE);
	}

	int CFixShareMemory::Read(int iPacketIndex, BYTE* pDataBuff, int iBuffLen, int* pDataLen)
	{
		return Read(iPacketIndex, pDataBuff, iBuffLen, pDataLen, INFINITE);
	}

}

