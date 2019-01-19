#ifndef __SYS_UTILITIES_REPLYLOCK_H__
#define __SYS_UTILITIES_REPLYLOCK_H__

#include "SysUtilities.h"
#include "Cond.h"

namespace SYS_UTL
{
	/**
	* \brief ����Ķ�д��..
	*	һ���̼߳�����ִ�����Ȼ��ȴ�ĳ����������
	*	һ���̵߳õ�����ִ�н������������.
	*	����Ӧ�ó���������ͨ���У����������ִ�У��ȴ�����ִ����ɡ�
	*
	*	�Զ������ͷ��� [CAutoRepLock]
	*/
	class SYS_UTL_CPPAPI CReplyLock
	{
		CReplyLock(CReplyLock const &) = delete;
		CReplyLock(CReplyLock &&) = delete;
		CReplyLock& operator= (CReplyLock const &) = delete;
		CReplyLock& operator= (CReplyLock &&) = delete;
	public:
		CReplyLock();
		~CReplyLock();
		/**
		* \brief ����
		*/
		bool Lock();
		/**
		* \brief ����
		* \param uiTimeOut ��ʱʱ��(����)
		*/
		bool TryLock(DWORD dwTimeOut = 100);
		/**
		* \brief �ͷ���
		*/
		void UnLock();

		/**
		* \brief ������������
		*	���� Signal ǰ,���� Lock.	
		*/
		int Signal();

		/**
		* \brief �ȴ�ִ�лظ�
		*	ֻ���� Lock �ɹ��󣬲��� Wait
		*/
		bool Wait();
		/**
		* \brief �ȴ�ִ�лظ�
		*	ֻ���� Lock �ɹ��󣬲��� TryWait
		* \param uiTimeOut ��ʱʱ��(����)
		*/
		bool TryWait(DWORD dwTimeOut = 100);

	private:
		/**< �ź���.*/
		SYS_UTL::CCond m_Cond;
		/**< �ٽ���Դ.*/
		SYS_UTL::CCritSec m_Sec;

	};
}

#endif	//__AUTOLOCK_H__
