#ifndef __SYS_UTILITIES_THREADBOX_H__
#define __SYS_UTILITIES_THREADBOX_H__
#include "CritSec.h"

namespace SYS_UTL
{
	typedef void(*BOX_THREAD_PROCESS)(BOOL& bRun, HANDLE hWait, void* pUserContext);
	/**
	* \brief �̺߳з�װ��
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
		* \brief ��ʼ��. 
		* \param iPacketSize ��������С
		* \param bPacket �Ƿ�ÿ��д��,���ȡ�ᰴÿ�����.
		* \param bCreateEvent �Ƿ񴴽��¼�,�������ݰ�д�룬�򼤻��¼�.
		*/
		int Init(int iPacketSize = 1024, BOOL bPacket = TRUE, BOOL bCreateEvent = TRUE);
		int Init(int iPacketSize, BOOL bPacket, BOOL bCreateEvent, const char* lpName);
		void UnInit();
		/**
		* \brief �����߳�.
		*	�ڲ�����ʹ�� BOX_THREAD_PROCESS �ص�,�ٴ� function�ص�,
		* \param lpProcess �ص�
		*/
		int Start(SYS_UTL::BOX_THREAD_PROCESS lpProcess, void* pUserContext);
		int Start(std::function<void(BOOL& bRun, HANDLE hWait, void* context)>);
		int Stop(DWORD dwTimeOut = 60000);
		/**
		* \brief д������.
		* \param pData ���ݻ�����
		* \param iDataLen ���ݳ���
		* \param bEvent TRUE,����ȴ��¼�
		*/
		int WriteData(void* pData, int iDataLen, BOOL bEvent = TRUE);
		int WriteDataEx(void* pData[], int iDataLen[], int iNum, BOOL bEvent = TRUE);
		/**
		* \brief ��ȡ����.
		* \param pData ������
		* \param iDataLen ����������
		* \param lpRequestSize ���������Ȳ�������£��������Ҫ�Ļ�������С
		*/
		int ReadData(void* pData, int iDataLen, int* lpRequestSize = NULL, BOOL bForceGet = FALSE);
		/**
		* \brief �߳��Ƿ�����.
		*/
		BOOL IsStart();
		/**
		* \brief �߳��Ƿ�������.
		*/
		BOOL IsData();
		/**
		* \brief �������ݶ�дλ��.
		*/
		void ResetData();
		/**
		* \brief �����߳�ֹͣ.
		*/
		int Cancel();

		int OpenEvent();
		void CloseEvent();
		void SetEvent();
		HANDLE GetEventHandle();
		void ResetEvent();
		/**
		* \brief �߳�ID.
		*/
		DWORD GetCurrentThreadId();
		const char* GetThreadName();

	protected:
		static DWORD WINAPI Process(void* lpThis);

	protected:
		/// �ͷų�ʼ��
		BOOL m_bInit;
		/// �߳̾��
		HANDLE m_hObject;
		/// �¼����
		HANDLE m_hWait;
		BOOL m_bDone;
		BOOL m_bRun;
		/// �߳�ID
		DWORD m_dwID;
		/// �Ƿ񴴽��¼�
		BOOL m_bCreateEvent;
		/// �����Ƿ񰴰�д��.
		BOOL m_bPacket;

		SYS_UTL::BOX_THREAD_PROCESS	m_pProcess;
		void* m_ProcessContext;
		std::function<void(BOOL& bRun, HANDLE hWait, void* context)> m_cb;

		/**
		* \brief ����������
		*/
		/// ������ָ��
		BYTE* m_pBuffer;
		/// ��������С
		int m_iBufferLen;
		/// ������д��λ��
		int m_iWritePos;
		/// ��������ȡλ��
		int m_iReadPos;
		/// ���������ݴ�С
		int m_iDataLen;

		SYS_UTL::CCritSec m_mutexPacket;
		/**�̱߳�ʶ*/
		char m_szName[64];
	};
}

#endif //__THREADBOX_H__
