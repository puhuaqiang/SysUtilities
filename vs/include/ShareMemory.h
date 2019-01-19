#ifndef __SYS_UTILITIES_SHARE_MEMORY_H__
#define __SYS_UTILITIES_SHARE_MEMORY_H__

#include "SysUtilities.h"
#include "Mutex.h"
#include "Event.h"
#include "ThreadBox.h"

/***
* \brief �����ڴ��װ��
*
* CShareMemory
*	�����ڴ�����ѭ��д����д������ͨ���ص�����֪ͨ, ���ݲ����ظ���ȡ
*
* CFixShareMemory
*	��̬���ݵĹ����ڴ棬д�����ݺ����ݿ��ظ���ȡ
*/
namespace SYS_UTL
{
#define PROC_FLAG 0x0f0a0b0c
	/// ����ͨ�����ݰ�.
	struct CommunicatorPacket_t
	{
		DWORD dwFlag; //PROC_FLAG
		DWORD dwType; //���ݰ�����
		int iLen; //���ݳ���
		BYTE bData[4];	//���ݶ�
	};
	/// ����ͨѶ������.
	enum CommunicatorType {
		CT_READER = 0,	//ֻ��
		CT_WRITER	//д��
	};
	typedef struct _SHARE_MEMORY_PROPERTY
	{
		/// �������ݶ�����.
		char szShareSegName[128];
		/// ���ݱ���¼�����.
		char szChangeEventName[128];
		/// ���ݶλ���������.
		char szMutexName[128];
	}SHARE_MEMORY_PROPERTY, *PSHARE_MEMORY_PROPERTY;

	class SYS_UTL_CPPAPI CShareMemory
	{
	public:
		CShareMemory();
		~CShareMemory();

		/**
		* \brief ��ʼ������ͨѶ��
		* \param nType ����.
		* \param lpProperty ����
		* \param dwLength �����ڴ���󳤶�
		* \return
		*/
		int	Init(CommunicatorType nType, PSHARE_MEMORY_PROPERTY lpProperty,DWORD dwLength);
		/**
		* \brief �ͷ���Դ
		*/
		void UnInit();
		/**
		* \brief ע���ȡ��������д������ݵĻص���������ʼ��ΪCT_READERʱ��Ч
		*	�ڲ������ж� �ص�����,���ж� std::function
		*/
		void RegisterDataCallback(void(*pCallback)(CommunicatorPacket_t* lpPacket, void* pUsrPar), void* pUsrPar);
		void RegisterDataCallback(std::function<void(CommunicatorPacket_t* lpPacket)>);

		/**
		* \brief ������д�빲���棬��ʼ��ΪCT_WRITERʱ��Ч
		*/
		int WriteData(CommunicatorPacket_t* pData);

		/**
		* \brief ������ݣ���ʼ��ΪCT_READERʱ��Ч
		*/
		void ResetData();

	protected:
		/**
		* \brief ���ݶ�ȡ�̴߳�����
		*/
		static void  ReadDataThread(BOOL& bRun, HANDLE hWait, void* pUsrPar);
		void ReadData(BOOL& bRun, HANDLE hWait);

		BOOL __IsInit() const { return m_bInit; }
	protected:
		/// �Ƿ��Ѿ���ʼ��.
		BOOL m_bInit; 
		/// ͨѶ������.
		CommunicatorType m_nType;
		/// ����
		HANDLE m_hFileMapping;
		/// ���ݱ��֪ͨ.
		SYS_UTL::CEventWrap m_WriteEvent;
		/// ���ݻ�����.
		SYS_UTL::CMutexWrap m_Mutex;
		/// ���ݶ�ȡ�̣߳���������Ϊ��ȡģʽʱ����
		SYS_UTL::CThreadBox m_read_thr;
		/// ��������ص�����
		std::function<void(CommunicatorPacket_t* lpPacket)> m_cb;
		struct
		{
			void(*lpFun)(CommunicatorPacket_t* lpPacket, void* pUsrPar);
			void* lpUsr;
		}m_stCallBack;
		/// �����ڴ����ַ.
		BYTE* m_lpBasePointer;
	};

	class SYS_UTL_CPPAPI CFixShareMemory
	{
	public:
		CFixShareMemory();
		~CFixShareMemory();

		/**
		* \brief ��ʼ������ͨѶ��
		* \param nType ����.
		* \param lpProperty ����
		* \param dwLength �����ڴ���󳤶�
		* \return
		*/
		int	Init(CommunicatorType nType, const char* lpszShareMemoryName, int iMaxPerPacketSize, int iMaxPacketCount);
		/**
		* \brief �ͷ���Դ
		*/
		void UnInit();

		/**
		* \brief ������д�빲���棬��ʼ��ΪCT_WRITERʱ��Ч
		* \param iPacketIndex ���ݱ�����.
		* \param pDataBuff ���ݻ�����.
		* \param iDataLen ���ݴ�С.
		* \return 0�ɹ�,����ʧ��
		*/
		int Write(int iPacketIndex, BYTE* pDataBuff, int iDataLen);

		/**
		* \brief ������д�빲���棬��ʼ��ΪCT_READERʱ��Ч
		* \param iPacketIndex ���ݱ�����.
		* \param pDataBuff ������.
		* \param iBuffLen ��������С.
		* \param pDataLen ���ݴ�С.
		* \return 0�ɹ�,����ʧ��
		*/
		int Read(int iPacketIndex, BYTE* pDataBuff, int iBuffLen, int* pDataLen);

		/**
		* \brief ������д�빲���棬��ʼ��ΪCT_WRITERʱ��Ч
		* \param iPacketIndex ���ݱ�����.
		* \param pDataBuff ���ݻ�����.
		* \param iDataLen ���ݴ�С.
		* \param dwTimeOut ��ʱʱ��.
		* \return 0�ɹ�,����ʧ��
		*/
		int Write(int iPacketIndex, BYTE* pDataBuff, int iDataLen, DWORD dwTimeOut);

		/**
		* \brief ������д�빲���棬��ʼ��ΪCT_READERʱ��Ч
		* \param iPacketIndex ���ݱ�����.
		* \param pDataBuff ������.
		* \param iBuffLen ��������С.
		* \param pDataLen ���ݴ�С.
		* \param dwTimeOut ��ʱʱ��
		* \return 0�ɹ�,����ʧ��
		*/
		int Read(int iPacketIndex, BYTE* pDataBuff, int iBuffLen, int* pDataLen, DWORD dwTimeOut);

	protected:

		BOOL __IsInit() const { return m_bInit; }
	protected:
		/// �Ƿ��Ѿ���ʼ��.
		BOOL m_bInit;
		/// ͨѶ������.
		CommunicatorType m_nType;
		/// ����
		HANDLE m_hFileMapping;
		/// ���ݻ�����.
		SYS_UTL::CMutexWrap m_Mutex;
		/// �����ڴ����ַ.
		BYTE* m_lpBasePointer;
		int m_iMaxPerPacketSize;
		int m_iMaxPacketCount;
	};

} // End of  namespace section

#endif