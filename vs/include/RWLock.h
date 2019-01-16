#ifndef __SYS_UTILITIES_WRLOCK_H__
#define __SYS_UTILITIES_WRLOCK_H__

#include "SysUtilities.h"
#include "CritSec.h"
#include "Sem.h"

namespace SYS_UTL
{
	/**
	* \brief ��д����װ��.
	*/
	class SYS_UTL_CPPAPI CRWLock
	{
		CRWLock(CRWLock const &) = delete;
		CRWLock(CRWLock &&) = delete;
		CRWLock& operator= (CRWLock const &) = delete;
		CRWLock& operator= (CRWLock &&) = delete;
	public:
		CRWLock();
		~CRWLock();
		/**
		* \brief �Ӷ���
		* \return bool ������� true ����������� false
		*/
		bool RLock();
		/**
		* \brief ���ԼӶ���
		* \param uiTimeOut ��ʱʱ��(����)
		* \return bool ������� true ����������� false
		*/
		bool TryRLock(DWORD dwTimeOut = 0);
		/**
		* \brief �ͷŶ���
		*/
		void RUnLock();
		/**
		* \brief ��д��
		* \return bool ������� true ����������� false
		*/
		bool WLock();
		/**
		* \brief ���Լ�д��
		* \return bool ������� true ����������� false
		*/
		bool TryWLock(DWORD dwTimeOut = 0);
		/**
		* \brief �ͷ�д��
		*/
		void WUnLock();
	private:
		void __Init();
		void __UnInit();
		bool __IsInit() const;
	private:
		/**< �ͷų�ʼ��.*/
		bool m_Init;
		/**< �ź���.*/
		SYS_UTL::CSem m_Sem;
		/**< ��������.*/
		unsigned int m_uiReaders;
		/**< ��.�ٽ���Դ.*/
		SYS_UTL::CCritSec m_Sec;
	};
}

#endif	//__AUTOLOCK_H__
