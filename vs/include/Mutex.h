#ifndef __SYS_UTILITIES_MUTEX_H__
#define __SYS_UTILITIES_MUTEX_H__

#include "SysUtilities.h"
#include "AutoLock.h"

namespace SYS_UTL
{
	/***
	* \brief �¼���װ��
	*/
	class SYS_UTL_CPPAPI CMutexWrap
	{
		CMutexWrap(const CMutexWrap &ref);
		CMutexWrap &operator=(const CMutexWrap &ref);
	public:
		CMutexWrap();
		~CMutexWrap();
		/**
		* \brief ��ʼ��.
		* \param bManualReset �ֶ���λ�����Զ���λ������TRUE��ʾ�ֶ���λ������FALSE��ʾ�Զ���λ��
		* \param bInitialState �¼��ĳ�ʼ״̬������TRUR��ʾ�Ѵ�����
		* \param lpName �¼������ƣ�����NULL��ʾ�����¼�
		* \param bOpen �Ƿ���Ѵ��ڵ��¼�
		*/
		bool Init(BOOL bInitialOwner, const char* lpName, bool bOpen = false);
		bool Init(LPSECURITY_ATTRIBUTES, BOOL bInitialOwner, const char* lpName, bool bOpen = false);
		/**
		* \brief �ͷ���Դ.
		*/
		void UnInit();

		/**
		* \brief �ȴ��ź�.
		*	���� true ����ʾ�źű�����, ����ʧ��.
		*/
		bool Wait();

		/**
		* \brief �ȴ��ź�.
		* \param iTimeOut ��ʱʱ��(����)
		* \return ����TRUE ��ʾ�����ɹ�,�������ʧ��
		*/
		bool TryWait(DWORD dwTimeOut = 500);
	
		/**
		* \brief �����ź�.
		*	���ؼ���ʾ�����ɹ�
		*/
		bool Release();

		HANDLE Get();
	private:
		bool __Valid() const;
	protected:
		/**�ź���*/
		HANDLE m_hMutex;
	};

	class SYS_UTL_CPPAPI CAutoMutex
	{
		CAutoMutex(const CAutoMutex &ref);
		CAutoMutex &operator=(const CAutoMutex &ref);

	public:
		/**
		* \brief 
		*/
		CAutoMutex(SYS_UTL::CMutexWrap *lpMutex);
		/**
		* \brief 
		*/
		CAutoMutex(SYS_UTL::CMutexWrap *lpMutex, SYS_UTL::LOCK_FLAG::__adopt_lock_t);
		CAutoMutex(SYS_UTL::CMutexWrap *lpMutex, SYS_UTL::LOCK_FLAG::__defer_lock_t);

		~CAutoMutex();

		/**
		* \brief �Ƿ�����ɹ�.
		* \return ����TRUE,�����ɹ�, �������ʧ��.
		*/
		BOOL Owns() const;

		/**
		* \brief �ȴ�ִ�лظ�
		*	ֻ���� Lock �ɹ��󣬲��� Wait
		*/
		BOOL Wait();
		/**
		* \brief �ȴ�ִ�лظ�
		*	ֻ���� Lock �ɹ��󣬲��� TryWait
		* \param uiTimeOut ��ʱʱ��(����)
		*/
		BOOL TryWait(DWORD dwTimeOut = 100);

	private:
		SYS_UTL::CMutexWrap* m_lpMutex;
		/**�Ƿ�ӵ����*/
		BOOL m_owns;
	};
}

#endif	//__CRITSEC_H__
