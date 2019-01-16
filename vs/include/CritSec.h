#ifndef __SYS_UTILITIES_CRITSEC_H__
#define __SYS_UTILITIES_CRITSEC_H__

#include "SysUtilities.h"

namespace SYS_UTL
{
	/***
	* \brief �ٽ�����װ��
	*	1.֧�ֳ�ʱ����.
	*	2.֧�ַǼ����߳�ͨ���������,֪ͨ�Ѽ����߳��ͷ�����
	*/
	class SYS_UTL_CPPAPI CCritSec
	{
		CCritSec(const CCritSec &refAutoLock);
		CCritSec &operator=(const CCritSec &refAutoLock);
	public:
		CCritSec();
		~CCritSec();
		/**
		* \brief ����.
		*	���ؼ���ʾ�����ɹ�
		*/
		void Lock();

		/**
		* \brief ����.
		* \param iTimeOut ��ʱʱ��
		* \return ����TRUE ��ʾ�����ɹ�,�������ʧ��
		*/
		bool LockWait(int iTimeOut = 500);
		/**
		* \brief ����.
		* \param bWait ѭ���ȴ���ʶ
		* \param iTimeOut ��ʱʱ��
		* \return ����TRUE ��ʾ�����ɹ�,�������ʧ��
		*/
		bool LockWaitEx(bool& bWait, int iTimeOut = 500);
		/**
		* \brief �ͷ���.
		*	���ؼ���ʾ�ͷ����ɹ�
		*/
		void Unlock();

		/**
		* \brief Ҫ�����̼���.�ýӿ���Ҫ�ڼ���ǰ����
		*	�ýӿڱ����� [ReleaseRequireFlag]���ʹ��,
		*	˭(�߳�) [RequireLock] �ɹ�����Ҫ���� [ReleaseRequireFlag]�ͷ�
		* \return ������� FALSE,��ʶ����ʧ��,˵���Ѿ���ĳ���̱߳�ʶ��
		*/
		bool RequireLock();
		/**
		* \brief �ͷ� ǿ�Ƽ�����ʶ
		*/
		void ReleaseRequireFlag();
		/**
		* \brief ǿ�Ƽ�����ʶ �Ƿ�����
		*	�ǽ����߳��жϵ���ʶ����������Ҫ�����ͷ���.
		*/
		bool IsRequire();

	protected:
		/**�ٽ���ʵ��*/
		CRITICAL_SECTION m_CritSec;
		/**
		* \brief ĳ�̼߳�����,��������̵߳�.
		*	������ʶ�߳��Ѿ�ǿ����,
		*	�����ǽ����̼߳�����,��Ҫ�жϸñ�ʶ,����ñ�ʶ������,��ǽ����߳���Ҫ�����ͷ���.
		*/
		BOOL m_bUrgentNeed;
	};
}

#endif	//__CRITSEC_H__
