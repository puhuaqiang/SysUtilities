#ifndef __SYS_UTILITIES_EVENT_H__
#define __SYS_UTILITIES_EVENT_H__

#include "SysUtilities.h"

namespace SYS_UTL
{
	/***
	* \brief 事件封装类
	*/
	class SYS_UTL_CPPAPI CEventWrap
	{
		CEventWrap(const CEventWrap &ref);
		CEventWrap &operator=(const CEventWrap &ref);
	public:
		CEventWrap();
		~CEventWrap();
		/**
		* \brief 初始化.
		* \param bManualReset 手动置位还是自动置位，传入TRUE表示手动置位，传入FALSE表示自动置位。
		* \param bInitialState 事件的初始状态，传入TRUR表示已触发。
		* \param lpName 事件的名称，传入NULL表示匿名事件
		* \param bOpen 是否打开已存在的事件
		*/
		bool Init(BOOL bManualReset, BOOL bInitialState,const char* lpName, bool bOpen = false);
		bool Init(LPSECURITY_ATTRIBUTES, BOOL bManualReset, BOOL bInitialState, const char* lpName, bool bOpen = false);
		/**
		* \brief 释放资源.
		*/
		void UnInit();

		/**
		* \brief 等待信号.
		*	返回 true 即表示信号被触发, 否则失败.
		*/
		bool Wait();

		/**
		* \brief 等待信号.
		* \param iTimeOut 超时时间(毫秒)
		* \return 返回TRUE 表示加锁成功,否则加锁失败
		*/
		bool TryWait(DWORD dwTimeOut = 500);
	
		/**
		* \brief 触发信号.
		*	返回即表示触发成功
		*/
		bool Set();

		/**
		* \brief 将事件设为末触发.
		*/
		bool ReSet();

		/**
		* \brief 将事件触发后立即将事件设置为未触发，相当于触发一个事件脉冲
		* 如果是一个人工重置事件：把event对象设置为激活状态，唤醒"所有"等待中的线程，然后event恢复为非激活状态
		* 如果是一个自动重置事件：把event对象设置为激活状态，唤醒"一个"等待中的线程，然后event恢复为非激活状态
		*/
		bool Pulse();

		HANDLE Get();
	private:
		bool __Valid() const;
	protected:
		/**信号量*/
		HANDLE m_hEvent;
	};
}

#endif	//__CRITSEC_H__
