#ifndef __SYS_UTILITIES_SHARE_MEMORY_H__
#define __SYS_UTILITIES_SHARE_MEMORY_H__

#include "SysUtilities.h"
#include "Mutex.h"
#include "Event.h"
#include "ThreadBox.h"

/***
* \brief 共享内存封装类
*
* CShareMemory
*	共享内存数据循环写读，写入数据通过回调函数通知, 数据不可重复读取
*
* CFixShareMemory
*	静态数据的共享内存，写入数据后，数据可重复读取
*/
namespace SYS_UTL
{
#define PROC_FLAG 0x0f0a0b0c
	/// 进程通信数据包.
	struct CommunicatorPacket_t
	{
		DWORD dwFlag; //PROC_FLAG
		DWORD dwType; //数据包类型
		int iLen; //数据长度
		BYTE bData[4];	//数据段
	};
	/// 进程通讯器类型.
	enum CommunicatorType {
		CT_READER = 0,	//只读
		CT_WRITER	//写入
	};
	typedef struct _SHARE_MEMORY_PROPERTY
	{
		/// 共享数据段名称.
		char szShareSegName[128];
		/// 数据变更事件名称.
		char szChangeEventName[128];
		/// 数据段互斥锁名称.
		char szMutexName[128];
	}SHARE_MEMORY_PROPERTY, *PSHARE_MEMORY_PROPERTY;

	class SYS_UTL_CPPAPI CShareMemory
	{
	public:
		CShareMemory();
		~CShareMemory();

		/**
		* \brief 初始化进程通讯器
		* \param nType 类型.
		* \param lpProperty 属性
		* \param dwLength 共享内存最大长度
		* \return
		*/
		int	Init(CommunicatorType nType, PSHARE_MEMORY_PROPERTY lpProperty,DWORD dwLength);
		/**
		* \brief 释放资源
		*/
		void UnInit();
		/**
		* \brief 注册获取共享缓存中写入的数据的回调函数，初始化为CT_READER时有效
		*	内部优先判断 回调函数,再判断 std::function
		*/
		void RegisterDataCallback(void(*pCallback)(CommunicatorPacket_t* lpPacket, void* pUsrPar), void* pUsrPar);
		void RegisterDataCallback(std::function<void(CommunicatorPacket_t* lpPacket)>);

		/**
		* \brief 将数据写入共享缓存，初始化为CT_WRITER时有效
		*/
		int WriteData(CommunicatorPacket_t* pData);

		/**
		* \brief 清空数据，初始化为CT_READER时有效
		*/
		void ResetData();

	protected:
		/**
		* \brief 数据读取线程处理函数
		*/
		static void  ReadDataThread(BOOL& bRun, HANDLE hWait, void* pUsrPar);
		void ReadData(BOOL& bRun, HANDLE hWait);

		BOOL __IsInit() const { return m_bInit; }
	protected:
		/// 是否已经初始化.
		BOOL m_bInit; 
		/// 通讯器类型.
		CommunicatorType m_nType;
		/// 命名
		HANDLE m_hFileMapping;
		/// 内容变更通知.
		SYS_UTL::CEventWrap m_WriteEvent;
		/// 数据互斥锁.
		SYS_UTL::CMutexWrap m_Mutex;
		/// 数据读取线程，仅当创建为读取模式时创建
		SYS_UTL::CThreadBox m_read_thr;
		/// 数据输出回调函数
		std::function<void(CommunicatorPacket_t* lpPacket)> m_cb;
		struct
		{
			void(*lpFun)(CommunicatorPacket_t* lpPacket, void* pUsrPar);
			void* lpUsr;
		}m_stCallBack;
		/// 共享内存基地址.
		BYTE* m_lpBasePointer;
	};

	class SYS_UTL_CPPAPI CFixShareMemory
	{
	public:
		CFixShareMemory();
		~CFixShareMemory();

		/**
		* \brief 初始化进程通讯器
		* \param nType 类型.
		* \param lpProperty 属性
		* \param dwLength 共享内存最大长度
		* \return
		*/
		int	Init(CommunicatorType nType, const char* lpszShareMemoryName, int iMaxPerPacketSize, int iMaxPacketCount);
		/**
		* \brief 释放资源
		*/
		void UnInit();

		/**
		* \brief 将数据写入共享缓存，初始化为CT_WRITER时有效
		* \param iPacketIndex 数据表索引.
		* \param pDataBuff 数据缓冲区.
		* \param iDataLen 数据大小.
		* \return 0成功,否则失败
		*/
		int Write(int iPacketIndex, BYTE* pDataBuff, int iDataLen);

		/**
		* \brief 将数据写入共享缓存，初始化为CT_READER时有效
		* \param iPacketIndex 数据表索引.
		* \param pDataBuff 缓冲区.
		* \param iBuffLen 缓冲区大小.
		* \param pDataLen 数据大小.
		* \return 0成功,否则失败
		*/
		int Read(int iPacketIndex, BYTE* pDataBuff, int iBuffLen, int* pDataLen);

		/**
		* \brief 将数据写入共享缓存，初始化为CT_WRITER时有效
		* \param iPacketIndex 数据表索引.
		* \param pDataBuff 数据缓冲区.
		* \param iDataLen 数据大小.
		* \param dwTimeOut 超时时间.
		* \return 0成功,否则失败
		*/
		int Write(int iPacketIndex, BYTE* pDataBuff, int iDataLen, DWORD dwTimeOut);

		/**
		* \brief 将数据写入共享缓存，初始化为CT_READER时有效
		* \param iPacketIndex 数据表索引.
		* \param pDataBuff 缓冲区.
		* \param iBuffLen 缓冲区大小.
		* \param pDataLen 数据大小.
		* \param dwTimeOut 超时时间
		* \return 0成功,否则失败
		*/
		int Read(int iPacketIndex, BYTE* pDataBuff, int iBuffLen, int* pDataLen, DWORD dwTimeOut);

	protected:

		BOOL __IsInit() const { return m_bInit; }
	protected:
		/// 是否已经初始化.
		BOOL m_bInit;
		/// 通讯器类型.
		CommunicatorType m_nType;
		/// 命名
		HANDLE m_hFileMapping;
		/// 数据互斥锁.
		SYS_UTL::CMutexWrap m_Mutex;
		/// 共享内存基地址.
		BYTE* m_lpBasePointer;
		int m_iMaxPerPacketSize;
		int m_iMaxPacketCount;
	};

} // End of  namespace section

#endif