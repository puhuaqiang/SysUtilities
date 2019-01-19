#ifndef __SYS_UTILITIES_EVENT_H__
#define __SYS_UTILITIES_EVENT_H__

#include "SysUtilities.h"

namespace SYS_UTL
{
	/***
	* \brief �¼���װ��
	*/
	class SYS_UTL_CPPAPI CEventWrap
	{
		CEventWrap(const CEventWrap &ref);
		CEventWrap &operator=(const CEventWrap &ref);
	public:
		CEventWrap();
		~CEventWrap();
		/**
		* \brief ��ʼ��.
		* \param bManualReset �ֶ���λ�����Զ���λ������TRUE��ʾ�ֶ���λ������FALSE��ʾ�Զ���λ��
		* \param bInitialState �¼��ĳ�ʼ״̬������TRUR��ʾ�Ѵ�����
		* \param lpName �¼������ƣ�����NULL��ʾ�����¼�
		* \param bOpen �Ƿ���Ѵ��ڵ��¼�
		*/
		bool Init(BOOL bManualReset, BOOL bInitialState,const char* lpName, bool bOpen = false);
		bool Init(LPSECURITY_ATTRIBUTES, BOOL bManualReset, BOOL bInitialState, const char* lpName, bool bOpen = false);
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
		bool Set();

		/**
		* \brief ���¼���Ϊĩ����.
		*/
		bool ReSet();

		/**
		* \brief ���¼��������������¼�����Ϊδ�������൱�ڴ���һ���¼�����
		* �����һ���˹������¼�����event��������Ϊ����״̬������"����"�ȴ��е��̣߳�Ȼ��event�ָ�Ϊ�Ǽ���״̬
		* �����һ���Զ������¼�����event��������Ϊ����״̬������"һ��"�ȴ��е��̣߳�Ȼ��event�ָ�Ϊ�Ǽ���״̬
		*/
		bool Pulse();

		HANDLE Get();
	private:
		bool __Valid() const;
	protected:
		/**�ź���*/
		HANDLE m_hEvent;
	};
}

#endif	//__CRITSEC_H__
