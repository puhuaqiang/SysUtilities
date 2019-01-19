#ifndef __SYS_UTILITIES_SEMAPHORE_H__
#define __SYS_UTILITIES_SEMAPHORE_H__

#include "SysUtilities.h"

namespace SYS_UTL
{
	/***
	* \brief �ź�����װ��
	*	1.֧�ֳ�ʱ����.
	*/
	class SYS_UTL_CPPAPI CSem
	{
		CSem(const CSem &refAutoLock);
		CSem &operator=(const CSem &refAutoLock);
	public:
		CSem();
		~CSem();
		/**
		* \brief ��ʼ��.
		* \param uiSem ��ʼ��Դ����
		* \param uiMax ��󲢷�����
		* \param lpName �ź��������ƣ�����NULL��ʾ�����ź���
		* \param bOpen �Ƿ���Ѵ��ڵ��ź���
		*/
		bool Init(unsigned int uiSem, unsigned int uiMax, const char* lpName = NULL, bool bOpen = false);
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
		*	���ؼ���ʾ�����źųɹ�
		*/
		bool Post();

	private:
		bool __Valid() const;
	protected:
		/**�ź���*/
		HANDLE m_hSem;
	};
}

#endif	//__CRITSEC_H__
