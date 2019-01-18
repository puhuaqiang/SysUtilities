#ifndef __SYS_UTILITIES_CONDITION_H__
#define __SYS_UTILITIES_CONDITION_H__

#include "CritSec.h"
#include "Sem.h"

namespace SYS_UTL
{
	/**
	* \brief �ȵ�ָ����Ŀ���߳�һ�𵽴����һ�����ִ��
	*/
	class SYS_UTL_CPPAPI CCond
	{
		CCond(const CCond &ref);
		CCond &operator=(const CCond &ref);

	public:
		CCond();
		~CCond();
		/**
		* \brief ��ʼ��
		*/
		int Init();
		/**
		* \brief �ͷ���Դ
		*/
		void UnInit();

		/**
		* ������������,����һ���ȴ��е��߳�
		*/
		int Signal();

		/**
		* ������������,���ѵȴ��е������߳�
		*/
		int Broadcast();

		/**
		* ������������������
		*/
		bool Wait();

		/**
		* ������������������
		* /param dwTimeOut ��ʱʱ��(����)
		*/
		bool TryWait(DWORD dwTimeOut);
		/**
		* \brief �Ƿ��ʼ���ɹ�
		*/
		BOOL IsInit() const;

		typedef union _COND_PROPERTY
		{
			CONDITION_VARIABLE cond_var;
			struct {
				unsigned int waiters_count;
				CRITICAL_SECTION waiters_count_lock;
				HANDLE signal_event;
				HANDLE broadcast_event;
			} fallback;
		}COND_PROPERTY, *LPLCOND_PROPERTY;

	private:
		/// �Ƿ��ʼ���ɹ�
		BOOL m_bInit;
		SYS_UTL::CCritSec m_Mutex;
		COND_PROPERTY m_Cond;
	};

}

#endif	//__AUTOLOCK_H__
