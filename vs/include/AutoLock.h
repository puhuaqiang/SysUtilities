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
	* \brief 临界区自动加/释放锁
	*/
	class SYS_UTL_CPPAPI CAutoLock
	{
		CAutoLock(const CAutoLock &refAutoLock);
		CAutoLock &operator=(const CAutoLock &refAutoLock);

	public:
		/**
		* \brief 强制加锁
		* \param plock 临界区实例
		*/
		CAutoLock(SYS_UTL::CCritSec *plock);
		/**
		* \brief 强制加锁
		* \param plock 临界区实例
		*/
		CAutoLock(SYS_UTL::CCritSec *plock, SYS_UTL::LOCK_FLAG::__adopt_lock_t);
		/**
		* \brief 延迟加锁
		* \param plock 临界区实例
		*/
		CAutoLock(SYS_UTL::CCritSec *plock, SYS_UTL::LOCK_FLAG::__defer_lock_t);
		/**
		* \brief 延迟加锁，并激活强制加锁标识
		* \param plock 临界区实例
		*/
		CAutoLock(SYS_UTL::CCritSec *plock, SYS_UTL::LOCK_FLAG::__defer_req_lock_t);
		~CAutoLock();

		/**
		* \brief 是否加锁成功.
		* \return 返回TRUE,加锁成功, 否则加锁失败.
		*/
		BOOL Owns() const;

		/**
		* \brief 加锁.
		* \return 返回TRUE,加锁成功, 否则加锁失败.
		*/
		BOOL Lock();

		/**
		* \brief 加锁.
		* \param dwTimeOut 超时返回.
		* \return 返回TRUE,加锁成功, 否则加锁失败.
		*/
		BOOL Lock(DWORD dwTimeOut);

		/**
		* \brief 是否有其他线程要求[强制加锁标识].
		* \return 返回TRUE,该实例已要求, 否则未要求.
		*/
		BOOL IsOtherRequire() const;

		/**
		* \brief 当前实例是否要求[强制加锁标识].
		* \return 返回TRUE,该实例已要求, 否则未要求.
		*/
		BOOL IsRequire() const;

		/**
		* \brief 当前实例是否激活[强制加锁标识]成功.
		* \return 返回TRUE,激活成功, 否则激活失败.
		*/
		BOOL IsRequireSucc() const;

	private:
		void __Initialize();
		/**
		* \brief 判断当前实例是否 要求[强制加锁标识]
		*	如果未要求返回TRUE
		*	如果已要求，则判断是否激活[强制加锁标识]成功
		*	如果激活成功返回TRUE,否则返回FALSE
		*/
		BOOL __CheckRequireFlag();

	private:
		SYS_UTL::CCritSec* m_lpLock;
		/**是否拥有锁*/
		BOOL m_owns;
		/**请求强制加锁*/
		BOOL m_bRequire;
		/**激活[强制加锁标识]是否成功*/
		BOOL m_bRequireSucc;
	};


	/**
	* \brief 自定加/释放 [读写锁]
	*/
	class SYS_UTL_CPPAPI CAutoRWLock
	{
		CAutoRWLock(const CAutoRWLock &refAutoLock);
		CAutoRWLock &operator=(const CAutoRWLock &refAutoLock);

	public:
		/**
		* \brief 读锁
		*/
		CAutoRWLock(SYS_UTL::CRWLock *plock, SYS_UTL::LOCK_FLAG::__adopt_read_lock_t);
		/**
		* \brief 写锁
		*/
		CAutoRWLock(SYS_UTL::CRWLock *plock, SYS_UTL::LOCK_FLAG::__adopt_write_lock_t);
		/**
		* \brief 延迟读锁
		*/
		CAutoRWLock(SYS_UTL::CRWLock *plock, SYS_UTL::LOCK_FLAG::__defer_read_lock_t);
		/**
		* \brief 延迟写锁
		*/
		CAutoRWLock(SYS_UTL::CRWLock *plock, SYS_UTL::LOCK_FLAG::__defer_write_lock_t);
		~CAutoRWLock();

		/**
		* \brief 是否加锁成功.
		* \return 返回TRUE,加锁成功, 否则加锁失败.
		*/
		BOOL Owns() const;

		/**
		* \brief 加锁.
		* \return 返回TRUE,加锁成功, 否则加锁失败.
		*/
		BOOL Lock();

		/**
		* \brief 加锁.
		* \param dwTimeOut 超时返回.
		* \return 返回TRUE,加锁成功, 否则加锁失败.
		*/
		BOOL Lock(DWORD dwTimeOut);

	private:
		SYS_UTL::CRWLock* m_lpLock;
		/**是否拥有锁*/
		BOOL m_owns;
		/**请求强制加锁*/
		enum RWLOCK_TYPE
		{
			RWLOCK_TYPE_NONE = 0,
			RWLOCK_TYPE_READ = 1,
			RWLOCK_TYPE_WRITE = 2,
		};
		RWLOCK_TYPE m_nLockType;
	};

	/**
	* \brief 自定加/释放 [锁]
	*/
	class SYS_UTL_CPPAPI CAutoRepLock
	{
		CAutoRepLock(const CAutoRepLock &refAutoLock);
		CAutoRepLock &operator=(const CAutoRepLock &refAutoLock);

	public:
		/**
		* \brief 读锁
		*/
		CAutoRepLock(SYS_UTL::CReplyLock *plock, SYS_UTL::LOCK_FLAG::__adopt_lock_t);
		/**
		* \brief 延迟读锁
		*/
		CAutoRepLock(SYS_UTL::CReplyLock *plock, SYS_UTL::LOCK_FLAG::__defer_lock_t);
		~CAutoRepLock();

		/**
		* \brief 是否加锁成功.
		* \return 返回TRUE,加锁成功, 否则加锁失败.
		*/
		BOOL Owns() const;

		/**
		* \brief 加锁.
		* \return 返回TRUE,加锁成功, 否则加锁失败.
		*/
		BOOL Lock();

		/**
		* \brief 加锁.
		* \param dwTimeOut 超时返回.
		* \return 返回TRUE,加锁成功, 否则加锁失败.
		*/
		BOOL Lock(DWORD dwTimeOut);

		/**
		* \brief 等待执行回复
		*	只有在 Lock 成功后，才能 Wait
		*/
		BOOL Wait();
		/**
		* \brief 等待执行回复
		*	只有在 Lock 成功后，才能 TryWait
		* \param uiTimeOut 超时时间(毫秒)
		*/
		BOOL TryWait(DWORD dwTimeOut = 100);

	private:
		SYS_UTL::CReplyLock* m_lpLock;
		/**是否拥有锁*/
		BOOL m_owns;
	};
}

#endif	//__AUTOLOCK_H__
