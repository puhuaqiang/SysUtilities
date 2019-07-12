#ifndef __SYS_UTILITIES_NET_NET_H__
#define __SYS_UTILITIES_NET_NET_H__
#include "InetAddress.h"
#include "../SysUtilities.h"


/**
* @brief ����ͨ�ŷ�װ.
* 
* ע��: �ڲ�û�м���,�ⲿ�������м���.
*/
namespace SYS_UTL{namespace NET{
/**
* @brief �׽���ͨ����Э�����.
* ..
*/
enum TRANS_PROTOCOL_TYPE
{
	TRANS_PROTOCOL_TYPE_NONE = 0,
	/**< ����Э��ʹ��TCP.*/
	TRANS_PROTOCOL_TYPE_TCP = 1,
	/**< ����Э��ʹ��UDP.*/
	TRANS_PROTOCOL_TYPE_UDP = 2
};


/**
* @brief �׽���ͨ�Ż���.
* ʹ��[select]ģ��
*/
class SYS_UTL_CPPAPI CNet
{
protected:
	CNet();
	CNet(SYS_UTL::NET::TRANS_PROTOCOL_TYPE);
	/**
	* @brief �׽���ͨ����Э�����.
	*/
	CNet(SOCKET skt, SYS_UTL::NET::TRANS_PROTOCOL_TYPE);
	virtual ~CNet() = 0;
public:

	/**
	@brief �ر��׽���.
	*/
	virtual void Close();

	/**
	* @brief �Ƿ���Ч�׽���.
	* 	skt != INVALID_SOCKET
	*/
	virtual bool IsValidSkt() const;

	/**
	@brief ����socket api�Ƿ��ʹ���.
	*/
	virtual bool IsError() const;

	/**
	@brief ��ȡ���紫��Э��.
	*/
	SYS_UTL::NET::TRANS_PROTOCOL_TYPE GetTransProtocol() const;

	/**
	* @brief ��ȡUDP�Զ˵�ַ��Ϣ.
	* @param stAddr ���ջ�����
	*/
	virtual int GetPeerAddrInfo(struct sockaddr& stAddr);

	/**
	* @brief �����׽���ѡ��.
	* 	�����μ�windows��[setsockopt]
	*/
	int SetSktOpt(int level, int optname, const char* optval, int optlen);

	/**
	* @brief �����׽���ѡ��.
	* 	�����μ�windows��[getsockopt]
	*/
	int GetSktOpt(int level, int optname, char* optval, int *optlen);

	/**
	@brief �����׽��ַ�����ģʽ.
	*/
	int SetSktNoBlock();
	/**
	* @brief ����TCP�׽��ֵ�Nagle�㷨.
	* 	��ѡ����Ĭ������±����ã�����ΪFALSE����
	*/
	int SetTcpNoDelay();

	/**
	* @brief ����ÿ���׽��ַ��ͻ������ռ�����.
	* 	Ĭ�ϴ�С��8k
	* 	�������λ0,������ֱ�ӷ��͵�ϵͳ������
	*/
	int SetSktSendBufferSize(unsigned int uiSize);

	/**
	* @brief ����ÿ���׽��ֽ��ջ������ռ�����.
	* 	Ĭ�ϴ�С��8k
	*/
	int SetSktRecvBufferSize(unsigned int uiSize);

	/**
	* @brief �׽��ִ��ݹرճ�ʱ.
	* @param uiTime ��ʱ(��). 0-��ֹ���ݹر�.
	*/
	int SetSktCloseLinger(unsigned int uiTime);

	/**
	* @brief �����׽��ְ󶨵�����ʹ�õĵ�ַ�Ͷ˿�.
	* 	SO_EXCLUSIVEADDRUSEѡ����Է�ֹ�������.
	* @param bReuse true-����.
	*/
	int SetSktReuseAddr(bool bReuse);

	/**
	* @brief ��д����..
	* @param uiTimeOut ��ʱʱ��,=0,Ҳ�ᱣ֤[select]���һ��
	*/
	unsigned int Read(char* pBuff, int iBuffLen, const int iReadLen = 0, unsigned int uiTimeOut = 5000);
	unsigned int ReadFromUDP(char* pBuff, int iBuffLen, struct sockaddr& from, int fromlen, unsigned int uiTimeOut = 5000);
	unsigned int Write(const char* pBuff, int iBuffLen, unsigned int uiTimeOut = 5000);
	unsigned int WriteToUDP(const char* pBuff, int iBuffLen, struct sockaddr& to, int tolen, unsigned int uiTimeOut = 5000);
protected:
	/**
	* @brief ���ô�����.
	* @param iErrorCode ������
	* @param lien ����λ��
	*/
	void SetErrorCode(int iErrorCode, int line);
protected:
	/**< ͨ���׽���.*/
	SOCKET m_Skt;
	/**< ������.*/
	int m_ErrorCode;
	/**< ����Э������.*/
	SYS_UTL::NET::TRANS_PROTOCOL_TYPE m_nTransProType;
};


/**
* @brief ����ͨ�ſͻ���..
* ʹ��[select]ģ��
*/
class SYS_UTL_CPPAPI CNetClient
	: public CNet
{
public:
	CNetClient();
	CNetClient(SYS_UTL::NET::TRANS_PROTOCOL_TYPE);
	CNetClient(SOCKET skt, SYS_UTL::NET::TRANS_PROTOCOL_TYPE);
	CNetClient(SOCKET skt, SYS_UTL::NET::TRANS_PROTOCOL_TYPE, InetAddress local, InetAddress peer);
	virtual ~CNetClient();

	/**
	* @brief �����׽���.
	* 	�����׽���ʹ��.�ڲ��ᴴ��\�����׽���
	* @param nType �μ�TRANS_PROTOCOL_TYPE
	* @param host IP��ַ. �󶨵���IP��ַ host == nullptr�� ��ʹ��[INADDR_ANY]
	* @param port �˿�
	* @param error ������
	* @param bConn ���UDP��Ч,���[true]��UDP����˽�������,[false]����������
	* @return int ������Ч����Ч�׽���
	*/
	int ConnectSocket(TRANS_PROTOCOL_TYPE nType, const char* host, unsigned short port, int* error = nullptr, bool bConn = true);

	/**
	* @brief �����׽���. �ӿ��ڲ��Զ�����Ϊ [TRANS_PROTOCOL_TYPE_TCP]
	* @param skt ��Ч�׽���
	*/
	void Attach(SOCKET skt);

	const InetAddress& GetPeerAddr();
private:
	InetAddress m_local;
	InetAddress m_peer;
};


/**
* @brief ����ͨ�ŷ�����..
* ʹ��[select]ģ��
*/
class SYS_UTL_CPPAPI CNetServer
	: public CNet
{
public:
	CNetServer();
	CNetServer(SYS_UTL::NET::TRANS_PROTOCOL_TYPE);
	virtual ~CNetServer();

	/**
	* @brief ���������׽���.
	* @	�������˴����׽���ʹ��.�ڲ��ᴴ��\��\�����׽���
	* @param nType �μ�TRANS_PROTOCOL_TYPE
	* @param host �󶨵���IP��ַ host == nullptr�� ��ʹ��[INADDR_ANY]
	* @param port �˿�
	* @param error ������
	*/
	int OpenSocket(TRANS_PROTOCOL_TYPE nType, const char* host, unsigned short port, int* error = nullptr);

	/**
	* @brief ���տͻ�������.
	* @param pClientIns ���ӳɹ�,���ؿͻ���ʵ��
	* @return bool �пͻ������ӷ���true,���򷵻�false
	*/
	bool Accept(CNetClient*& pClientIns);
	/**
	* @brief ���տͻ�������.
	* @return int �пͻ������ӷ�����Ч socket,���򷵻�INVALID_SOCKET
	*/
	SOCKET Accept();

};
//int ShutdownSocket(int sock);

/**
* @brief ��TCPЭ���׽����϶�ȡ����.
* @	��С��ʱʱ�� 10����
* @param skt ��Ч���׽���
* @param pBuff �������ݻ�����.
* @param iBuffLen �������ݻ�������С
* @param iReadLen ��Ҫ��ȡ�����ݴ�С
* @param error ���������
* @param uiTimeOut ��ʱʱ��, =0,Ҳ�ᱣ֤[select]���һ��
* @return ��ȡ�����ݴ�С
*/
unsigned int ReadFromTcp(SOCKET skt, char* pBuff, int iBuffLen, const int iReadLen = 0, int* error = nullptr, unsigned int uiTimeOut = 5000);
/**
* @brief ��UDPЭ���׽����϶�ȡ����.
* @	��С��ʱʱ�� 10����
* @param skt ��Ч���׽���
* @param pBuff �������ݻ�����.
* @param iBuffLen �������ݻ�������С
* @param from �Զ˵�ַ��Ϣ
* @param fromlen ��ַ��Ϣ��С
* @param error ���������
* @param uiTimeOut ��ʱʱ��,=0,Ҳ�ᱣ֤[select]���һ��
* @return ��ȡ�����ݴ�С
*/
unsigned int ReadFromUdp(SOCKET skt, char* pBuff, int iBuffLen, struct sockaddr& from, int fromlen, int* error = NULL, unsigned int uiTimeOut = 5000);
/**
* @brief ��TCPЭ���׽�����д������.
* @	��С��ʱʱ�� 10����
* @param skt ��Ч���׽���
* @param pBuff �������ݻ�����.
* @param iBuffLen �������ݻ�������С
* @param error ���������
* @param uiTimeOut ��ʱʱ��,=0,Ҳ�ᱣ֤[select]���һ��
* @return д������ݴ�С
*/
unsigned int WriteFromTcp(SOCKET skt, const char* pBuff, int iBuffLen, int* error = NULL, unsigned int uiTimeOut = 5000);
/**
* @brief ��UDPЭ���׽�����д������.
* 	��С��ʱʱ�� 10����
* @param skt ��Ч���׽���
* @param pBuff �������ݻ�����.
* @param iBuffLen �������ݻ�������С
* @param from �Զ˵�ַ��Ϣ
* @param fromlen ��ַ��Ϣ��С
* @param error ���������
* @param uiTimeOut ��ʱʱ��,=0,Ҳ�ᱣ֤[select]���һ��
* @return д������ݴ�С
*/
unsigned int WriteFromUDP(SOCKET skt, const char* pBuff, int iBuffLen, struct sockaddr& to, int tolen, int* error = NULL, unsigned int uiTimeOut = 5000);

/**
@brief ��ʼ���׽���ʹ�û���,ʹ��Winsock DLL.
*/
SYS_UTL_CAPI int InitSocket();

/**
@brief
*/
SYS_UTL_CAPI int UnInitSocket();

/**
@brief �ر��׽���
*/
SYS_UTL_CAPI int CloseSocket(SOCKET);

/**
* @brief ���������׽���.
* 	�������˴����׽���ʹ��.�ڲ��ᴴ��\��\�����׽���
* @param nType �μ�TRANS_PROTOCOL_TYPE
* @param host host. �󶨵���IP��ַ host == nullptr�� ��ʹ��[INADDR_ANY]
* @param port �˿�
* @param error ������
* @return int ������Ч����Ч�׽���
*/
SOCKET OpenSocket(TRANS_PROTOCOL_TYPE nType, const char* host, unsigned short port, int* error = NULL);

/**
* @brief �����׽���. �ͻ��˴����׽���ʹ��.�ڲ��ᴴ��\�����׽���
* @param nType �μ�TRANS_PROTOCOL_TYPE
* @param host IP��ַ. �󶨵���IP��ַ host == nullptr�� ��ʹ��[INADDR_ANY]
* @param port �˿�
* @param error ������
* @param bConn ���UDP��Ч,���[true]��UDP����˽�������,[false]����������
* @return int ������Ч����Ч�׽���
*/
SOCKET ConnectSocket(TRANS_PROTOCOL_TYPE nType, const char* host, unsigned short port, int* error = NULL, bool bConn = true);

/**
* @brief �����׽���.
* @param skt ��Ч���׽���
* @param pid ��ʹ�ø����׽��ֵ�Ŀ����̵Ľ��̱�ʶ��.
* @param pProtocolInfo ָ���ɿͻ��˷���Ļ�������ָ�룬���С���԰���WSAPROTOCOL_INFO�ṹ.
* @param iBuffLen ��������С
* @param pDataLen ���ݴ�С
*/
SYS_UTL_CAPI int DuplicateSocket(int skt, unsigned int pid, char* pProtocolInfo, int iBuffLen, int* pDataLen);

/**
* @brief ����Э����Ϣ�ṹ����,�����׽���.
* @param pProtocolInfo Э����Ϣ�ṹ
* @param iBuffLen ��������С
* @param skt ������׽���
*/
SYS_UTL_CAPI int CreateSocketFromDuplicate(char* pProtocolInfo, int iDataLen, int& skt);

/**
* @brief �����׽��������ӵĶԵ���ĵ�ַ.
* @param skt ��Ч�׽���
* @param ip ���IP��ַ�Ļ�����
* @param len ��������С
* @param port �˿�
*/
SYS_UTL_CAPI int GetSktPeerInfo(int skt, char* ip, int len, int* port);

/**
* @brief �����׽��ֵı�������.
* @param skt ��Ч�׽���
* @param ip ���IP��ַ�Ļ�����
* @param len ��������С
* @param port �˿�
*/
SYS_UTL_CAPI int GetSktName(int skt, char* ip, int len, int* port);

/**
* @brief ��ȡ�ڵ��IP���˿�.
* @param addr �ڵ��ַ��Ϣ
* @param ip ������
* @param port ������
*/
SYS_UTL_CAPI int GetSktNameEx(sockaddr& addr, char* ip, int len, int* port);

/**
* @brief ���[sockaddr]�ṹ��.
* @param ip ���IP��ַ�Ļ�����
* @param port �˿�
* @param addr ���[sockaddr]�ṹ��.������
*/
SYS_UTL_CAPI int StuffSockAddr(TRANS_PROTOCOL_TYPE nType, char* ip, int port, sockaddr& addr);
} //namespace SYS_UTL::NET
} // namespace SYS_UTL
#endif