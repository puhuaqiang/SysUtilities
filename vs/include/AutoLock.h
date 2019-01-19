#ifndef __SYS_UTILITIES_AUTOLOCK_H__
#define __SYS_UTILITIES_AUTOLOCK_H__

#include "CritSec.h"
#include "RWLock.h"
#include "ReplyLock.h"

namespace SYS_UTL
{
	namespace LOCK_FLAG
	{
		// LOCK PROPERTIES
		struct SYS_UTL_CPPAPI __adopt_lock_t
		{	// indicates adopt lock
		};

		struct SYS_UTL_CPPAPI __defer_lock_t
		{	// indicates defer lock
		};

		struct SYS_UTL_CPPAPI __defer_req_lock_t
		{	// indicates defer lock
		};

		const __adopt_lock_t lock_adopt;
		const __defer_lock_t lock_defer;
		const __defer_req_lock_t lock_defer_req;

		struct SYS_UTL_CPPAPI __adopt_read_lock_t
		{
		};
		struct SYS_UTL_CPPAPI __adopt_write_lock_t
		{
		};
		struct SYS_UTL_CPPAPI __defer_read_lock_t
		{
		};
		struct SYS_UTL_CPPAPI __defer_write_lock_t
		{
		};
		const __adopt_read_lock_t lock_read;
		const __adopt_write_lock_t lock_write;
		const __defer_read_lock_t lock_defer_read;
		const __defer_write_lock_t lock_defer_write;
	}

	/**
	* \brief �ٽ����Զ���/�ͷ���
	*/
	class SYS_UTL_CPPAPI CAutoLock
	{
		CAutoLock(const CAutoLock &refAutoLock);
		CAutoLock &operator=(const CAutoLock &refAutoLock);

	public:
		/**
		* \brief ǿ�Ƽ���
		* \param plock �ٽ���ʵ��
		*/
		CAutoLock(SYS_UTL::CCritSec *plock);
		/**
		* \brief ǿ�Ƽ���
		* \param plock �ٽ���ʵ��
		*/
		CAutoLock(SYS_UTL::CCritSec *plock, SYS_UTL::LOCK_FLAG::__adopt_lock_t);
		/**
		* \brief �ӳټ���
		* \param plock �ٽ���ʵ��
		*/
		CAutoLock(SYS_UTL::CCritSec *plock, SYS_UTL::LOCK_FLAG::__defer_lock_t);
		/**
		* \brief �ӳټ�����������ǿ�Ƽ�����ʶ
		* \param plock �ٽ���ʵ��
		*/
		CAutoLock(SYS_UTL::CCritSec *plock, SYS_UTL::LOCK_FLAG::__defer_req_lock_t);
		~CAutoLock();

		/**
		* \brief �Ƿ�����ɹ�.
		* \return ����TRUE,�����ɹ�, �������ʧ��.
		*/
		BOOL Owns() const;

		/**
		* \brief ����.
		* \return ����TRUE,�����ɹ�, �������ʧ��.
		*/
		BOOL Lock();

		/**
		* \brief ����.
		* \param dwTimeOut ��ʱ����.
		* \return ����TRUE,�����ɹ�, �������ʧ��.
		*/
		BOOL Lock(DWORD dwTimeOut);

		/**
		* \brief �Ƿ��������߳�Ҫ��[ǿ�Ƽ�����ʶ].
		* \return ����TRUE,��ʵ����Ҫ��, ����δҪ��.
		*/
		BOOL IsOtherRequire() const;

		/**
		* \brief ��ǰʵ���Ƿ�Ҫ��[ǿ�Ƽ�����ʶ].
		* \return ����TRUE,��ʵ����Ҫ��, ����δҪ��.
		*/
		BOOL IsRequire() const;

		/**
		* \brief ��ǰʵ���Ƿ񼤻�[ǿ�Ƽ�����ʶ]�ɹ�.
		* \return ����TRUE,����ɹ�, ���򼤻�ʧ��.
		*/
		BOOL IsRequireSucc() const;

	private:
		void __Initialize();
		/**
		* \brief �жϵ�ǰʵ���Ƿ� Ҫ��[ǿ�Ƽ�����ʶ]
		*	���δҪ�󷵻�TRUE
		*	�����Ҫ�����ж��Ƿ񼤻�[ǿ�Ƽ�����ʶ]�ɹ�
		*	�������ɹ�����TRUE,���򷵻�FALSE
		*/
		BOOL __CheckRequireFlag();

	private:
		SYS_UTL::CCritSec* m_lpLock;
		/**�Ƿ�ӵ����*/
		BOOL m_owns;
		/**����ǿ�Ƽ���*/
		BOOL m_bRequire;
		/**����[ǿ�Ƽ�����ʶ]�Ƿ�ɹ�*/
		BOOL m_bRequireSucc;
	};


	/**
	* \brief �Զ���/�ͷ� [��д��]
	*/
	class SYS_UTL_CPPAPI CAutoRWLock
	{
		CAutoRWLock(const CAutoRWLock &refAutoLock);
		CAutoRWLock &operator=(const CAutoRWLock &refAutoLock);

	public:
		/**
		* \brief ����
		*/
		CAutoRWLock(SYS_UTL::CRWLock *plock, SYS_UTL::LOCK_FLAG::__adopt_read_lock_t);
		/**
		* \brief д��
		*/
		CAutoRWLock(SYS_UTL::CRWLock *plock, SYS_UTL::LOCK_FLAG::__adopt_write_lock_t);
		/**
		* \brief �ӳٶ���
		*/
		CAutoRWLock(SYS_UTL::CRWLock *plock, SYS_UTL::LOCK_FLAG::__defer_read_lock_t);
		/**
		* \brief �ӳ�д��
		*/
		CAutoRWLock(SYS_UTL::CRWLock *plock, SYS_UTL::LOCK_FLAG::__defer_write_lock_t);
		~CAutoRWLock();

		/**
		* \brief �Ƿ�����ɹ�.
		* \return ����TRUE,�����ɹ�, �������ʧ��.
		*/
		BOOL Owns() const;

		/**
		* \brief ����.
		* \return ����TRUE,�����ɹ�, �������ʧ��.
		*/
		BOOL Lock();

		/**
		* \brief ����.
		* \param dwTimeOut ��ʱ����.
		* \return ����TRUE,�����ɹ�, �������ʧ��.
		*/
		BOOL Lock(DWORD dwTimeOut);

	private:
		SYS_UTL::CRWLock* m_lpLock;
		/**�Ƿ�ӵ����*/
		BOOL m_owns;
		/**����ǿ�Ƽ���*/
		enum RWLOCK_TYPE
		{
			RWLOCK_TYPE_NONE = 0,
			RWLOCK_TYPE_READ = 1,
			RWLOCK_TYPE_WRITE = 2,
		};
		RWLOCK_TYPE m_nLockType;
	};

	/**
	* \brief �Զ���/�ͷ� [��]
	*/
	class SYS_UTL_CPPAPI CAutoRepLock
	{
		CAutoRepLock(const CAutoRepLock &refAutoLock);
		CAutoRepLock &operator=(const CAutoRepLock &refAutoLock);

	public:
		/**
		* \brief ����
		*/
		CAutoRepLock(SYS_UTL::CReplyLock *plock, SYS_UTL::LOCK_FLAG::__adopt_lock_t);
		/**
		* \brief �ӳٶ���
		*/
		CAutoRepLock(SYS_UTL::CReplyLock *plock, SYS_UTL::LOCK_FLAG::__defer_lock_t);
		~CAutoRepLock();

		/**
		* \brief �Ƿ�����ɹ�.
		* \return ����TRUE,�����ɹ�, �������ʧ��.
		*/
		BOOL Owns() const;

		/**
		* \brief ����.
		* \return ����TRUE,�����ɹ�, �������ʧ��.
		*/
		BOOL Lock();

		/**
		* \brief ����.
		* \param dwTimeOut ��ʱ����.
		* \return ����TRUE,�����ɹ�, �������ʧ��.
		*/
		BOOL Lock(DWORD dwTimeOut);

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
		SYS_UTL::CReplyLock* m_lpLock;
		/**�Ƿ�ӵ����*/
		BOOL m_owns;
	};
}

#endif	//__AUTOLOCK_H__
