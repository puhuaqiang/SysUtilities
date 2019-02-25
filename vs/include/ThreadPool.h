#ifndef __SYS_UTILITIES_THREADPOOL_H__
#define __SYS_UTILITIES_THREADPOOL_H__

#include "ThreadBox.h"

namespace SYS_UTL
{
	typedef void(*THREAD_POOL_PROCESS)(void* lpTask, int iTaskDataLen, void* lpUsr);

	/**
	* \brief �̺߳з�װ��.
	*/
	class CThreadPool;
	class SYS_UTL_CPPAPI CThreadWrapper
	{
		friend class CThreadPool;
	private:
		CThreadWrapper();
		~CThreadWrapper();

	public:
		/**
		* \brief �����߳�.
		* \param lpProcCallBack ������ص�����
		* \param lpUsr �Զ���������
		*/
		int Start(int iPacketSize, SYS_UTL::THREAD_POOL_PROCESS lpProcCallBack, void* lpUsr);
		int Start(std::function<void(BOOL& bRun, HANDLE hWait, void* context)>, const char* lpszName = "");
		/**
		* \brief ֹͣ�߳�
		*/
		void Stop();

		/**
		* \brief �������
		* \param lpTask �������ݻ�����, ����Ϊ NULL, ���򷵻ز�������
		* \param iTaskDataLen �������ݴ�С
		*/
		int Task(void* lpTask, int iTaskDataLen);
		/**
		* \brief ��ȡ����ʱ��(�������ٺ���)
		*/
		int GetIdleTime();
		/**
		* \brief �Ƿ�������
		*/
		bool IsError();
		/**
		* \brief �Ƿ����״̬
		*/
		bool IsIdle();
		operator bool(){ return !IsError(); }
	private:
		/**
		* \brief
		*/
		void __UpdateIdleTime(unsigned int);
		/**
		* \brief
		*/
		void __UpdateError(int);
	private:
		/**�����߳�*/
		SYS_UTL::CThreadBox m_Thread;
		/**����ʱ���*/
		unsigned int m_uiIdleTimestamp;
		/**��*/
		SYS_UTL::CCritSec m_lock;
		/**��ȡ���񻺳���*/
		struct  {
			int iBufferLen;
			int iDataLen;
			BYTE buff[1024 * 8];
			BYTE* lpBuff;
		}m_Task;
		/**�ڲ���������*/
		int m_iError;
		/***/
		struct  {
			SYS_UTL::THREAD_POOL_PROCESS cb;
			void* lpUsr;
		}m_TaskCallBack;
		std::function<void(BOOL& bRun, HANDLE hWait, void* context)> m_cb;
	};

	/**
	* \brief �̳߳ط�װ��.
	*/
	class SYS_UTL_CPPAPI CThreadPool
	{
		CThreadPool(const CThreadPool &ref);
		CThreadPool &operator=(const CThreadPool &ref);
	public:
		CThreadPool();
		virtual ~CThreadPool();

		/**
		* \brief ��Դ��ʼ��.
		* \param iThreadCnt �̳߳��̸߳�������
		* \param iTaskBufferLen ���񻺳����ܴ�С(�ֽ�),С�ڵ���0x0FFFFFFF
		* \param iSingleTaskMaxLen �������񻺳�����󳤶�(�ֽ�), С�ڵ���iTaskBufferLen
		* \param iMaxIdleTime �߳�������ʱ��(����),�ﵽ������ʱ��,�̻߳��˳�. =0���������ڶ�����
		*/
		int Init(int iThreadCnt, int iTaskBufferLen, int iSingleTaskMaxLen, int iMaxIdleTime);
		/**
		* \brief ֹͣ�̳߳�
		*/
		void UnInit();

		/**
		* \brief �����̳߳�.
		*	�ڲ�����ʹ�� BOX_THREAD_PROCESS �ص�,�ٴ� function�ص�,
		* \param lpProcCallBack ������ص�����
		* \param lpUsr �Զ���������
		*/
		int Start(SYS_UTL::THREAD_POOL_PROCESS lpProcCallBack, void* lpUsr);
		int Start(std::function<void(void* lpTask, int iTaskDataLen)>);
		/**
		* \brief ֹͣ�̳߳�
		*/
		void Stop();

		/**
		* \brief �������
		* \param lpTask �������ݻ�����, ����Ϊ NULL, ���򷵻ز�������
		* \param iTaskDataLen �������ݴ�С
		* \return 0�ɹ� ,����ʧ��
		*/
		int Task(void* lpTask, int iTaskDataLen);

	private:

		/**�����̴߳�����*/
		static void __CtrlThreadProc(BOOL& bRun, HANDLE hWait, void* context);
		void __CtrlThreadProcLoop(BOOL& bRun, HANDLE hWait);
		/****/
		static void __ThreadCallBack(void* lpTask, int iTaskDataLen, void* lpUsr);
		void __ThreadCallBackProc(void* lpTask, int iTaskDataLen);

		/**
		* \brief �õ�һ���߳�ʵ��
		*/
		SYS_UTL::CThreadWrapper* __GetThread();

		/**
		* \brief ��⴦���߳�
		*	�������Ŀ���ʱ��,�ͷ��߳�
		*/
		void __CheckProcThread();

		/***/
		BOOL __IsInit() const { return m_bInit; }
		/***/
		BOOL __IsStartUp() const { return m_bStartUp; }

	private:
		/**�̳߳ؿ����߳�*/
		SYS_UTL::CThreadBox m_CtrlThread;
		/**�����̼߳���*/
		std::list<SYS_UTL::CThreadWrapper*> m_lstThreads;
		/**�Ƿ��ʼ��*/
		BOOL m_bInit;
		/**�Ƿ���������*/
		BOOL m_bStartUp;
		/**�߳�������ʱ��(����)*/
		int m_iMaxIdleTime;
		/**�̳߳�����߳���,����256*/
		int m_iMaxThreadCnt;
		int m_iSingleTaskMaxLen;
		SYS_UTL::CCritSec m_lock;
		/**������ص�����*/
		struct {
			SYS_UTL::THREAD_POOL_PROCESS lpPorc;
			void* lpUsr;
		}m_TaskCallBack;
		std::function<void(void* lpTask, int iTaskDataLen)> m_cb;
		/**��ȡ���񻺳���*/
		struct  {
			int iBufferLen;
			int iDataLen;
			BYTE buff[1024 * 8];
			BYTE* lpBuff;
		}m_Task;
	};
}

#endif