#ifndef __SYS_UTILITIES_NET_NET_H__
#define __SYS_UTILITIES_NET_NET_H__
#include "InetAddress.h"
#include "../SysUtilities.h"


/**
* @brief 网络通信封装.
* 
* 注意: 内部没有加锁,外部调用自行加锁.
*/
namespace SYS_UTL{namespace NET{
/**
* @brief 套接字通传输协议基类.
* ..
*/
enum TRANS_PROTOCOL_TYPE
{
	TRANS_PROTOCOL_TYPE_NONE = 0,
	/**< 传输协议使用TCP.*/
	TRANS_PROTOCOL_TYPE_TCP = 1,
	/**< 传输协议使用UDP.*/
	TRANS_PROTOCOL_TYPE_UDP = 2
};


/**
* @brief 套接字通信基类.
* 使用[select]模型
*/
class SYS_UTL_CPPAPI CNet
{
protected:
	CNet();
	CNet(SYS_UTL::NET::TRANS_PROTOCOL_TYPE);
	/**
	* @brief 套接字通传输协议基类.
	*/
	CNet(SOCKET skt, SYS_UTL::NET::TRANS_PROTOCOL_TYPE);
	virtual ~CNet() = 0;
public:

	/**
	@brief 关闭套接字.
	*/
	virtual void Close();

	/**
	* @brief 是否有效套接字.
	* 	skt != INVALID_SOCKET
	*/
	virtual bool IsValidSkt() const;

	/**
	@brief 调用socket api是否发送错误.
	*/
	virtual bool IsError() const;

	/**
	@brief 获取网络传输协议.
	*/
	SYS_UTL::NET::TRANS_PROTOCOL_TYPE GetTransProtocol() const;

	/**
	* @brief 获取UDP对端地址信息.
	* @param stAddr 接收缓存区
	*/
	virtual int GetPeerAddrInfo(struct sockaddr& stAddr);

	/**
	* @brief 设置套接字选项.
	* 	参数参见windows的[setsockopt]
	*/
	int SetSktOpt(int level, int optname, const char* optval, int optlen);

	/**
	* @brief 检索套接字选项.
	* 	参数参见windows的[getsockopt]
	*/
	int GetSktOpt(int level, int optname, char* optval, int *optlen);

	/**
	@brief 设置套接字非阻塞模式.
	*/
	int SetSktNoBlock();
	/**
	* @brief 禁用TCP套接字的Nagle算法.
	* 	该选项在默认情况下被禁用（设置为FALSE）。
	*/
	int SetTcpNoDelay();

	/**
	* @brief 设置每个套接字发送缓冲区空间总数.
	* 	默认大小是8k
	* 	如果设置位0,则数据直接发送到系统缓存区
	*/
	int SetSktSendBufferSize(unsigned int uiSize);

	/**
	* @brief 设置每个套接字接收缓冲区空间总数.
	* 	默认大小是8k
	*/
	int SetSktRecvBufferSize(unsigned int uiSize);

	/**
	* @brief 套接字从容关闭超时.
	* @param uiTime 超时(秒). 0-禁止从容关闭.
	*/
	int SetSktCloseLinger(unsigned int uiTime);

	/**
	* @brief 允许套接字绑定到已在使用的地址和端口.
	* 	SO_EXCLUSIVEADDRUSE选项可以防止这种情况.
	* @param bReuse true-允许.
	*/
	int SetSktReuseAddr(bool bReuse);

	/**
	* @brief 读写数据..
	* @param uiTimeOut 超时时间,=0,也会保证[select]检测一次
	*/
	unsigned int Read(char* pBuff, int iBuffLen, const int iReadLen = 0, unsigned int uiTimeOut = 5000);
	unsigned int ReadFromUDP(char* pBuff, int iBuffLen, struct sockaddr& from, int fromlen, unsigned int uiTimeOut = 5000);
	unsigned int Write(const char* pBuff, int iBuffLen, unsigned int uiTimeOut = 5000);
	unsigned int WriteToUDP(const char* pBuff, int iBuffLen, struct sockaddr& to, int tolen, unsigned int uiTimeOut = 5000);
protected:
	/**
	* @brief 设置错误码.
	* @param iErrorCode 错误码
	* @param lien 调用位置
	*/
	void SetErrorCode(int iErrorCode, int line);
protected:
	/**< 通信套接字.*/
	SOCKET m_Skt;
	/**< 错误码.*/
	int m_ErrorCode;
	/**< 传输协议类型.*/
	SYS_UTL::NET::TRANS_PROTOCOL_TYPE m_nTransProType;
};


/**
* @brief 网络通信客户端..
* 使用[select]模型
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
	* @brief 创建套接字.
	* 	创建套接字使用.内部会创建\连接套接字
	* @param nType 参见TRANS_PROTOCOL_TYPE
	* @param host IP地址. 绑定到的IP地址 host == nullptr， 则使用[INADDR_ANY]
	* @param port 端口
	* @param error 错误码
	* @param bConn 针对UDP有效,如果[true]与UDP服务端建立连接,[false]不建立连接
	* @return int 返回有效或无效套接字
	*/
	int ConnectSocket(TRANS_PROTOCOL_TYPE nType, const char* host, unsigned short port, int* error = nullptr, bool bConn = true);

	/**
	* @brief 关联套接字. 接口内部自动设置为 [TRANS_PROTOCOL_TYPE_TCP]
	* @param skt 有效套接字
	*/
	void Attach(SOCKET skt);

	const InetAddress& GetPeerAddr();
private:
	InetAddress m_local;
	InetAddress m_peer;
};


/**
* @brief 网络通信服务器..
* 使用[select]模型
*/
class SYS_UTL_CPPAPI CNetServer
	: public CNet
{
public:
	CNetServer();
	CNetServer(SYS_UTL::NET::TRANS_PROTOCOL_TYPE);
	virtual ~CNetServer();

	/**
	* @brief 创建监听套接字.
	* @	服务器端创建套接字使用.内部会创建\绑定\监听套接字
	* @param nType 参见TRANS_PROTOCOL_TYPE
	* @param host 绑定到的IP地址 host == nullptr， 则使用[INADDR_ANY]
	* @param port 端口
	* @param error 错误码
	*/
	int OpenSocket(TRANS_PROTOCOL_TYPE nType, const char* host, unsigned short port, int* error = nullptr);

	/**
	* @brief 接收客户端连接.
	* @param pClientIns 连接成功,返回客户端实例
	* @return bool 有客户端连接返回true,否则返回false
	*/
	bool Accept(CNetClient*& pClientIns);
	/**
	* @brief 接收客户端连接.
	* @return int 有客户端连接返回有效 socket,否则返回INVALID_SOCKET
	*/
	SOCKET Accept();

};
//int ShutdownSocket(int sock);

/**
* @brief 从TCP协议套接字上读取数据.
* @	最小超时时间 10毫秒
* @param skt 有效的套接字
* @param pBuff 接收数据缓存区.
* @param iBuffLen 接收数据缓存区大小
* @param iReadLen 需要读取的数据大小
* @param error 输出错误码
* @param uiTimeOut 超时时间, =0,也会保证[select]检测一次
* @return 读取的数据大小
*/
unsigned int ReadFromTcp(SOCKET skt, char* pBuff, int iBuffLen, const int iReadLen = 0, int* error = nullptr, unsigned int uiTimeOut = 5000);
/**
* @brief 从UDP协议套接字上读取数据.
* @	最小超时时间 10毫秒
* @param skt 有效的套接字
* @param pBuff 接收数据缓存区.
* @param iBuffLen 接收数据缓存区大小
* @param from 对端地址信息
* @param fromlen 地址信息大小
* @param error 输出错误码
* @param uiTimeOut 超时时间,=0,也会保证[select]检测一次
* @return 读取的数据大小
*/
unsigned int ReadFromUdp(SOCKET skt, char* pBuff, int iBuffLen, struct sockaddr& from, int fromlen, int* error = NULL, unsigned int uiTimeOut = 5000);
/**
* @brief 从TCP协议套接字上写入数据.
* @	最小超时时间 10毫秒
* @param skt 有效的套接字
* @param pBuff 接收数据缓存区.
* @param iBuffLen 接收数据缓存区大小
* @param error 输出错误码
* @param uiTimeOut 超时时间,=0,也会保证[select]检测一次
* @return 写入的数据大小
*/
unsigned int WriteFromTcp(SOCKET skt, const char* pBuff, int iBuffLen, int* error = NULL, unsigned int uiTimeOut = 5000);
/**
* @brief 从UDP协议套接字上写入数据.
* 	最小超时时间 10毫秒
* @param skt 有效的套接字
* @param pBuff 接收数据缓存区.
* @param iBuffLen 接收数据缓存区大小
* @param from 对端地址信息
* @param fromlen 地址信息大小
* @param error 输出错误码
* @param uiTimeOut 超时时间,=0,也会保证[select]检测一次
* @return 写入的数据大小
*/
unsigned int WriteFromUDP(SOCKET skt, const char* pBuff, int iBuffLen, struct sockaddr& to, int tolen, int* error = NULL, unsigned int uiTimeOut = 5000);

/**
@brief 初始化套接字使用环境,使用Winsock DLL.
*/
SYS_UTL_CAPI int InitSocket();

/**
@brief
*/
SYS_UTL_CAPI int UnInitSocket();

/**
@brief 关闭套接字
*/
SYS_UTL_CAPI int CloseSocket(SOCKET);

/**
* @brief 创建监听套接字.
* 	服务器端创建套接字使用.内部会创建\绑定\监听套接字
* @param nType 参见TRANS_PROTOCOL_TYPE
* @param host host. 绑定到的IP地址 host == nullptr， 则使用[INADDR_ANY]
* @param port 端口
* @param error 错误码
* @return int 返回有效或无效套接字
*/
SOCKET OpenSocket(TRANS_PROTOCOL_TYPE nType, const char* host, unsigned short port, int* error = NULL);

/**
* @brief 创建套接字. 客户端创建套接字使用.内部会创建\连接套接字
* @param nType 参见TRANS_PROTOCOL_TYPE
* @param host IP地址. 绑定到的IP地址 host == nullptr， 则使用[INADDR_ANY]
* @param port 端口
* @param error 错误码
* @param bConn 针对UDP有效,如果[true]与UDP服务端建立连接,[false]不建立连接
* @return int 返回有效或无效套接字
*/
SOCKET ConnectSocket(TRANS_PROTOCOL_TYPE nType, const char* host, unsigned short port, int* error = NULL, bool bConn = true);

/**
* @brief 复制套接字.
* @param skt 有效的套接字
* @param pid 将使用复制套接字的目标进程的进程标识符.
* @param pProtocolInfo 指向由客户端分配的缓冲区的指针，其大小足以包含WSAPROTOCOL_INFO结构.
* @param iBuffLen 缓存区大小
* @param pDataLen 数据大小
*/
SYS_UTL_CAPI int DuplicateSocket(int skt, unsigned int pid, char* pProtocolInfo, int iBuffLen, int* pDataLen);

/**
* @brief 根据协议信息结构内容,创建套接字.
* @param pProtocolInfo 协议信息结构
* @param iBuffLen 缓存区大小
* @param skt 分配的套接字
*/
SYS_UTL_CAPI int CreateSocketFromDuplicate(char* pProtocolInfo, int iDataLen, int& skt);

/**
* @brief 检索套接字所连接的对等体的地址.
* @param skt 有效套接字
* @param ip 存放IP地址的缓存区
* @param len 缓存区大小
* @param port 端口
*/
SYS_UTL_CAPI int GetSktPeerInfo(int skt, char* ip, int len, int* port);

/**
* @brief 检索套接字的本地名称.
* @param skt 有效套接字
* @param ip 存放IP地址的缓存区
* @param len 缓存区大小
* @param port 端口
*/
SYS_UTL_CAPI int GetSktName(int skt, char* ip, int len, int* port);

/**
* @brief 获取节点的IP、端口.
* @param addr 节点地址信息
* @param ip 缓存区
* @param port 缓存区
*/
SYS_UTL_CAPI int GetSktNameEx(sockaddr& addr, char* ip, int len, int* port);

/**
* @brief 填充[sockaddr]结构体.
* @param ip 存放IP地址的缓存区
* @param port 端口
* @param addr 输出[sockaddr]结构体.缓存区
*/
SYS_UTL_CAPI int StuffSockAddr(TRANS_PROTOCOL_TYPE nType, char* ip, int port, sockaddr& addr);
} //namespace SYS_UTL::NET
} // namespace SYS_UTL
#endif