#ifndef __SYS_UTILITIES_THREADBOX_H__
#define __SYS_UTILITIES_THREADBOX_H__
#include "CritSec.h"

namespace SYS_UTL
{
	typedef void(*BOX_THREAD_PROCESS)(BOOL& bRun, HANDLE hWait, void* pUserContext);
	/**
	* \brief 线程盒封装类
	*/
	class SYS_UTL_CPPAPI CThreadBox
	{
		CThreadBox(const CThreadBox &ref);
		CThreadBox &operator=(const CThreadBox &ref);
	public:
		CThreadBox();
		virtual ~CThreadBox();

		static bool Run(DWORD(WINAPI*)(void*), void* pParam);
		static bool RunEx(std::function<void(void* pParam)>, void* pParam);
		/// len <= 1024
		static bool RunEx2(std::function<void(void* lpData, int len, void* pParam)>, void* lpData, int len, void* pParam);
	public:
		/**
		* \brief 初始化. 
		* \param iPacketSize 缓冲区大小
		* \param bPacket 是否按每包写入,则读取会按每包输出.
		* \param bCreateEvent 是否创建事件,当有数据包写入，则激活事件.
		*/
		int Init(int iPacketSize = 1024, BOOL bPacket = TRUE, BOOL bCreateEvent = TRUE);
		int Init(int iPacketSize, BOOL bPacket, BOOL bCreateEvent, const char* lpName);
		void UnInit();
		/**
		* \brief 启动线程.
		*	内部优先使用 BOX_THREAD_PROCESS 回调,再次 function回调,
		* \param lpProcess 回调
		*/
		int Start(SYS_UTL::BOX_THREAD_PROCESS lpProcess, void* pUserContext);
		int Start(std::function<void(BOOL& bRun, HANDLE hWait, void* context)>);
		int Stop(DWORD dwTimeOut = 60000);
		/**
		* \brief 写入数据.
		* \param pData 数据缓存区
		* \param iDataLen 数据长度
		* \param bEvent TRUE,激活等待事件
		*/
		int WriteData(void* pData, int iDataLen, BOOL bEvent = TRUE);
		int WriteDataEx(void* pData[], int iDataLen[], int iNum, BOOL bEvent = TRUE);
		/**
		* \brief 读取数据.
		* \param pData 缓存区
		* \param iDataLen 缓存区长度
		* \param lpRequestSize 缓冲区长度不够情况下，会输出需要的缓冲区大小
		*/
		int ReadData(void* pData, int iDataLen, int* lpRequestSize = NULL, BOOL bForceGet = FALSE);
		/**
		* \brief 线程是否运行.
		*/
		BOOL IsStart();
		/**
		* \brief 线程是否有数据.
		*/
		BOOL IsData();
		/**
		* \brief 重置数据读写位置.
		*/
		void ResetData();
		/**
		* \brief 触发线程停止.
		*/
		int Cancel();

		int OpenEvent();
		void CloseEvent();
		void SetEvent();
		HANDLE GetEventHandle();
		void ResetEvent();
		/**
		* \brief 线程ID.
		*/
		DWORD GetCurrentThreadId();
		const char* GetThreadName();

	protected:
		static DWORD WINAPI Process(void* lpThis);

	protected:
		/// 释放初始化
		BOOL m_bInit;
		/// 线程句柄
		HANDLE m_hObject;
		/// 事件句柄
		HANDLE m_hWait;
		BOOL m_bDone;
		BOOL m_bRun;
		/// 线程ID
		DWORD m_dwID;
		/// 是否创建事件
		BOOL m_bCreateEvent;
		/// 数据是否按包写入.
		BOOL m_bPacket;

		SYS_UTL::BOX_THREAD_PROCESS	m_pProcess;
		void* m_ProcessContext;
		std::function<void(BOOL& bRun, HANDLE hWait, void* context)> m_cb;

		/**
		* \brief 缓冲区操作
		*/
		/// 缓冲区指针
		BYTE* m_pBuffer;
		/// 缓冲区大小
		int m_iBufferLen;
		/// 缓冲区写入位置
		int m_iWritePos;
		/// 缓冲区读取位置
		int m_iReadPos;
		/// 缓冲区数据大小
		int m_iDataLen;

		SYS_UTL::CCritSec m_mutexPacket;
		/**线程标识*/
		char m_szName[64];
	};
}

#endif //__THREADBOX_H__
