#include "../../include/net/SocketsOps.h"
#include "../../include/net/InetAddress.h"
#include "../../include/net/net.h"
#include <atomic>
#include <memory>
#include "../../include/api.h"
#include "../InDef.h"


namespace SYS_UTL{namespace NET{
struct ADDRINFOT_deleter
{
	void operator()(ADDRINFOA * ptr) const
	{
		FreeAddrInfoA(ptr);
	}

	void operator()(ADDRINFOW * ptr) const
	{
		FreeAddrInfoW(ptr);
	}
};

std::atomic_flag lock = ATOMIC_FLAG_INIT;
/**
* @brief 初始化启动进程使用Winsock DLL。
*	原子操作.
*/
void _InitSocket(){
	if (!lock.test_and_set()){
		WSADATA wsa;
		int ret = WSAStartup(MAKEWORD(2, 2), &wsa);
		if (ret != 0){
			lock.clear();
		}
	}
}
int InitSocket()
{
	_InitSocket();
	return 0;
}
int UnInitSocket()
{
	WSACleanup();
	return 0;
}
int CloseSocket(SOCKET skt)
{
	if (skt != INVALID_SOCKET)
	{
		::closesocket(skt);
	}
	return 0;
}
SOCKET OpenSocket(TRANS_PROTOCOL_TYPE nType, const char* host, unsigned short port, int* error /*= nullptr*/)
{
	if (nullptr != error)
	{
		*error = SOCKET_ERROR;
	}
#if 1
	int domain, type, protocol;
	domain = AF_INET;
	switch (nType)
	{
	case SYS_UTL::NET::TRANS_PROTOCOL_TYPE_TCP:
		type = SOCK_STREAM;
		protocol = IPPROTO_TCP;
		break;
	case SYS_UTL::NET::TRANS_PROTOCOL_TYPE_UDP:
		type = SOCK_DGRAM;
		protocol = IPPROTO_UDP;
		break;
	default:
		return INVALID_SOCKET;
	}

	SOCKET sockfd = socket(domain, type, protocol);
	if (sockfd == INVALID_SOCKET)
	{
		return INVALID_SOCKET;
	}
	InetAddress addr;
	if(NULL == host)
	{
		InetAddress addr1(port);
		addr = addr1;
	}else{
		InetAddress addr1(host,port);
		addr = addr1;
	}

	SYS_UTL::NET::SOCKETS::bindOrDie(sockfd, (const struct sockaddr*)addr.getSockAddr());

	if ((nType == SYS_UTL::NET::TRANS_PROTOCOL_TYPE_TCP))
	{
		SYS_UTL::NET::SOCKETS::listenOrDie(sockfd);
	}
	if (NULL != error)
	{
		*error = 0;
	}
	return sockfd;
#else
	ADDRINFOT stAddrInfoInts{};
	PADDRINFOT ai = nullptr;
	std::unique_ptr<ADDRINFOT, ADDRINFOT_deleter> plstAddrInfo;
	char szPort[32];
	_snprintf_s(szPort, _TRUNCATE, "%d", port);
	int iRetVal = 0;

	stAddrInfoInts.ai_family = AF_INET;
	switch (nType)
	{
	case SYS_UTL::NET::TRANS_PROTOCOL_TYPE_TCP:
		stAddrInfoInts.ai_socktype = SOCK_STREAM;
		stAddrInfoInts.ai_protocol = IPPROTO_TCP;
		break;
	case SYS_UTL::NET::TRANS_PROTOCOL_TYPE_UDP:
		stAddrInfoInts.ai_socktype = SOCK_DGRAM;
		stAddrInfoInts.ai_protocol = IPPROTO_UDP;
		break;
	default:
		return INVALID_SOCKET;
	}

	//AI_PASSIVE  - 当hostName为NULL时，给出ADDR_ANY和IN6ADDR_ANY_INIT
	stAddrInfoInts.ai_flags = AI_PASSIVE;
	iRetVal = GetAddrInfo(host, szPort, &stAddrInfoInts, &ai);
	if (iRetVal != 0){
		DBG_I("ERROR err:%d ERR:%d", iRetVal, WSAGetLastError());
		return INVALID_SOCKET;
	}

	plstAddrInfo.reset(ai);

#if 1
	SOCKET skt = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
#else
	SOCKET skt = WSASocketW(ai->ai_family, ai->ai_socktype, ai->ai_protocol, nullptr,
		0, WSA_FLAG_NO_HANDLE_INHERIT)
#endif
	if (skt == INVALID_SOCKET)
	{
		return INVALID_SOCKET;
	}
	if (bind(skt, ai->ai_addr, static_cast<int>(ai->ai_addrlen)) != 0)
	{
		DBG_I("ERROR ERR:%d", WSAGetLastError());
		closesocket(skt);
		return INVALID_SOCKET;
	}

	if ((nType == SYS_UTL::NET::TRANS_PROTOCOL_TYPE_TCP) && ::listen(skt, 16) != 0)
	{
		DBG_I("ERROR ERR:%d", WSAGetLastError());
		closesocket(skt);
		return INVALID_SOCKET;
	}
	if (nullptr != error)
	{
		*error = 0;
	}
	return skt;
#endif
}

SOCKET ConnectSocket(TRANS_PROTOCOL_TYPE nType, const char* host, unsigned short port, int* error /*= nullptr*/, bool bConn /*= true*/)
{
	if (nullptr != error)
	{
		*error = SOCKET_ERROR;
	}
#if 1
	int domain, type, protocol;
	domain = AF_INET;
	switch (nType)
	{
	case SYS_UTL::NET::TRANS_PROTOCOL_TYPE_TCP:
		type = SOCK_STREAM;
		protocol = IPPROTO_TCP;
		break;
	case SYS_UTL::NET::TRANS_PROTOCOL_TYPE_UDP:
		type = SOCK_DGRAM;
		protocol = IPPROTO_UDP;
		break;
	default:
		return INVALID_SOCKET;
	}
	SOCKET sockfd = socket(domain, type, protocol);
	if (sockfd == INVALID_SOCKET)
	{
		DBG_E;
		return INVALID_SOCKET;
	}
	InetAddress addr(host,port);
	int err = SYS_UTL::NET::SOCKETS::connectwrap(sockfd, addr.getSockAddr());
	if ( err == 0 )
	{
		if (NULL != error)
		{
			*error = 0;
		}
		return sockfd;
	}
	else if (SOCKET_ERROR == err)
	{
		::closesocket(sockfd);
		DBG_E;
		return INVALID_SOCKET;
	}

	fd_set writefds;
	struct timeval timeout;

	FD_ZERO(&writefds);
	FD_SET( sockfd, &writefds );

	timeout.tv_sec = 5; //timeout 
	timeout.tv_usec = 0;

	int ret = select(sockfd+1, NULL, &writefds, NULL, &timeout);
	if ( ret <= 0 )
	{
		DBG_E;
		::closesocket(sockfd);
		return INVALID_SOCKET;
	}

	if ( !FD_ISSET( sockfd, &writefds  ) )
	{
		DBG_E;
		::closesocket(sockfd);
		return INVALID_SOCKET;
	}

	/*err = 0;
	socklen_t length = sizeof( err );
	if( getsockopt( sockfd, SOL_SOCKET, SO_ERROR, &err, &length ) < 0 )
	{
		DBG_E;
		//printf( "get socket option failed\n" );
		close(sockfd);
		return -1;
	}

	if (err != 0)
	{
		DBG_E;
		//printf( "connection failed after select with the error: %d \n", error );
		close(sockfd);
		return -1;
	}*/
	if (NULL != error)
	{
		*error = 0;
	}
	return sockfd;
#else
	ADDRINFOT stAddrInfoInts{};
	PADDRINFOT ai = nullptr;
	std::unique_ptr<ADDRINFOT, ADDRINFOT_deleter> plstAddrInfo;
	char szPort[32];
	_snprintf_s(szPort, _TRUNCATE, "%d", port);
	int iRetVal = 0;

	stAddrInfoInts.ai_family = AF_INET;
	switch (nType)
	{
	case SYS_UTL::NET::TRANS_PROTOCOL_TYPE_TCP:
		stAddrInfoInts.ai_socktype = SOCK_STREAM;
		stAddrInfoInts.ai_protocol = IPPROTO_TCP;
		break;
	case SYS_UTL::NET::TRANS_PROTOCOL_TYPE_UDP:
		stAddrInfoInts.ai_socktype = SOCK_DGRAM;
		stAddrInfoInts.ai_protocol = IPPROTO_UDP;
		break;
	default:
		DBG_E;
		return INVALID_SOCKET;
	}

	stAddrInfoInts.ai_flags = AI_NUMERICSERV;
	iRetVal = GetAddrInfo(host, szPort, &stAddrInfoInts, &ai);
	if (iRetVal != 0)
	{
		DBG_I("ERROR ERR:%d", WSAGetLastError());
		return INVALID_SOCKET;
	}

	plstAddrInfo.reset(ai);
	iRetVal = 0;
	SOCKET skt = INVALID_SOCKET;
	for (ADDRINFOT * rp = ai; rp; rp = rp->ai_next)
	{
#if 1
		skt = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
#else
		skt = WSASocketW(rp->ai_family, rp->ai_socktype, rp->ai_protocol,
			nullptr, 0, WSA_FLAG_NO_HANDLE_INHERIT);
#endif
		if (skt == INVALID_SOCKET)
		{
			continue;
		}

		if (!bConn)
		{
			break;
		}
		while (((iRetVal = ::connect(skt, rp->ai_addr, static_cast<int>(rp->ai_addrlen))) == SOCKET_ERROR) && (WSAGetLastError() == WSAEINTR)){
			DBG_I("connect");
		}
		if (iRetVal == SOCKET_ERROR)
		{
			DBG_I("ERROR ERR:%d", WSAGetLastError());
			closesocket(skt);
			if (nullptr != error){
				*error = SOCKET_ERROR;
			}
			break;
		}
	}
	if ((iRetVal == 0) && (skt != INVALID_SOCKET))
	{
		if (nullptr != error)
		{
			*error = 0;
		}
	}
	else{
		DBG_I("ERROR ERR:%d ", WSAGetLastError());
	}
	return skt;
#endif
}

int DuplicateSocket(int iSocket, unsigned int pid, char* pProtocolInfo, int iBuffLen, int* pDataLen)
{
	return 0;
}

int CreateSocketFromDuplicate(char* pProtocolInfo, int iDataLen, int& skt)
{
	return 0;
}

int GetSktPeerInfo(int sockfd, char* ip, int len, int* port)
{
	InetAddress localAddr(SOCKETS::getPeerAddr(sockfd));
	strncpy_s(ip, len - 1, localAddr.toIp().data(), min(len-1, localAddr.toIp().length()));
	if (NULL != port)
	{
		*port = localAddr.toPort();
	}
	return 0;
}

int GetSktName(int sockfd, char* ip, int len, int* port)
{
	InetAddress localAddr(SOCKETS::getLocalAddr(sockfd));
	strncpy_s(ip, len - 1, localAddr.toIp().data(), min(len - 1, localAddr.toIp().length()));
	if (NULL != port)
	{
		*port = localAddr.toPort();
	}
	return 0;
}

int GetSktNameEx(sockaddr& addr, char* ip, int len, int* port)
{
	sockaddr_in* pInfo = (sockaddr_in*)&addr;
	if (port != NULL){
		*port = ntohs(pInfo->sin_port);
	}
	char szIP[64];
	szIP[0] = '\0';
	inet_ntop(AF_INET, &pInfo->sin_addr, szIP, 64);
	if (ip != NULL){
		strncpy_s(ip, len - 1, szIP, min(len-1, strlen(szIP)));
	}
	return 0;
}

int StuffSockAddr(TRANS_PROTOCOL_TYPE nType, char* ip, int port, sockaddr& addr)
{
	return -1;
}


unsigned int ReadFromTcp(SOCKET skt, char* pBuff, int iBuffLen, const int iReadLen/* = 0*/, int* error /*= nullptr*/, unsigned int uiTimeOut /*= 5000*/)
{
	int iDataLen = 0, iRet = 0, iError = 0;
	bool bRecvData = false; //是否有数据可以接收
	unsigned int uiCurTimeOut = GetTickCount();
	fd_set fdRead, fdExcep;
	struct timeval stTimeOut;
	if (NULL != error)
	{
		*error = iError;
	}
	if (NULL == pBuff || iBuffLen <= 0)
	{
		DBG_E;
		return -1;
	}
	do
	{
		//如果没有根据指定读取数据大小,则读取到多少数据就返回.
		if (iReadLen <= 0)
		{
			if (iDataLen > 0)
			{
				break;
			}
		}
		else{
			//已经读取到指定大小的数据
			if (iReadLen - iDataLen <= 0)
			{
				break;
			}
		}

		if (!bRecvData)
		{
			FD_ZERO(&fdRead);
			FD_ZERO(&fdExcep);
			FD_SET(skt, &fdRead);
			FD_SET(skt, &fdExcep);

			stTimeOut.tv_sec = 0;
			stTimeOut.tv_usec = 10000;
			iRet = select(-1/*skt + 1*/, &fdRead, NULL, &fdExcep, &stTimeOut);
			if (iRet > 0)
			{
				iRet = FD_ISSET(skt, &fdExcep);
				if (iRet)
				{//error
					iError = -1;
					DBG_E;
					break;
				}
				//是否有数据
				iRet = FD_ISSET(skt, &fdRead);
				if (iRet)
				{
					bRecvData = true;
				}
			}
			else{
				// iret == 0 超时
				if (iRet == SOCKET_ERROR)
				{
					iError = -2;
					DBG_E;
					DBG_I("SOCKET_ERROR err:%d", WSAGetLastError());
					break;
				}
			}
		}
		else {
			iRet = ::recv(skt, pBuff + iDataLen, iReadLen > 0 ? iReadLen - iDataLen : iBuffLen, 0);
			bRecvData = false;
			if (iRet > 0)
			{
				iDataLen += iRet;
			}
			if (iRet == 0)
			{//Server CloseSocket
				iError = -3;
				DBG_E;
				break;
			}
			if (iRet == SOCKET_ERROR)
			{
				int iErr = WSAGetLastError();
				if (iErr != WSAEWOULDBLOCK)
				{//没有数据
					iError = -4;
					DBG_I("SOCKET err:%d", WSAGetLastError());
					break;
				}
			}
		}

		if ((!bRecvData) && abs((int)(GetTickCount() - uiCurTimeOut)) >= (int)uiTimeOut)
		{
			//超时退出
			break;
		}

	} while (iDataLen < iBuffLen);
	if (NULL != error)
	{
		*error = iError;
	}
	return iDataLen;
}

unsigned int ReadFromUdp(SOCKET skt, char* pBuff, int iBuffLen, struct sockaddr& from, int fromlen, int* error /*= nullptr*/, unsigned int uiTimeOut /*= 5000*/)
{
	int iDataLen = 0, iRet = 0, iError = 0;
	bool bRecvData = false; //是否有数据可以接收
	unsigned int uiCurTimeOut = GetTickCount();
	fd_set fdRead, fdExcep;
	struct timeval stTimeOut;
	if (NULL != error)
	{
		*error = iError;
	}
	if (nullptr == pBuff || iBuffLen <= 0)
	{
		DBG_E;
		return -1;
	}
	do
	{
		if (iDataLen > 0)
		{
			//读取到数据直接返回.
			break;
		}

		if (!bRecvData)
		{
			FD_ZERO(&fdRead);
			FD_ZERO(&fdExcep);
			FD_SET(skt, &fdRead);
			FD_SET(skt, &fdExcep);

			stTimeOut.tv_sec = 0;
			stTimeOut.tv_usec = 10000;
			iRet = select(-1/*skt + 1*/, &fdRead, NULL, &fdExcep, &stTimeOut);
			if (iRet > 0)
			{
				iRet = FD_ISSET(skt, &fdExcep);
				if (iRet)
				{//error
					iError = -1;
					DBG_E;
					break;
				}
				//是否有数据
				iRet = FD_ISSET(skt, &fdRead);
				if (iRet)
				{
					bRecvData = true;
				}
				else{
					DBG_E;
				}
			}
			else{
				// iret == 0 超时
				if (iRet == SOCKET_ERROR)
				{
					iError = -2;
					DBG_E;
					break;
				}
				if (skt == INVALID_SOCKET)
				{
					iError = -3;
					DBG_E;
					break;
				}
			}
		}
		else {
			iRet = ::recvfrom(skt, pBuff, iBuffLen, 0, &from, (socklen_t*)&fromlen);
			bRecvData = false;
			if (iRet > 0)
			{
				iDataLen = iRet;
			}
			if (iRet == 0)
			{//Server CloseSocket
				iError = -3;
				DBG_E;
				break;
			}
			if (iRet == SOCKET_ERROR)
			{
				int iErr = WSAGetLastError();
				if (iErr != WSAEWOULDBLOCK)
				{//没有数据
					iError = SOCKET_ERROR;
					DBG_I("SOCKET err:%d", WSAGetLastError());
					break;
				}
			}
		}
		if ((!bRecvData) && abs((int)(GetTickCount() - uiCurTimeOut)) >= (int)uiTimeOut)
		{
			//超时退出
			break;
		}

	} while (iDataLen < iBuffLen);
	if (nullptr != error)
	{
		*error = iError;
	}
	return iDataLen;
}

unsigned int WriteFromTcp(SOCKET skt, const char* pBuff, int iBuffLen, int* error /*= nullptr*/, unsigned int uiTimeOut/* = 5000*/)
{
	int iSendLen = 0, iRet = 0, iError = 0;
	bool bSendData = true; //是否可以发送数据
	unsigned int uiCurTimeOut = GetTickCount();
	fd_set fdWrite, fdExcep;
	struct timeval stTimeOut;

	if (NULL != error)
	{
		*error = iError;
	}
	if (nullptr == pBuff || iBuffLen <= 0)
	{
		DBG_E;
		return -1;
	}
	do
	{
		if (iBuffLen - iSendLen <= 0)
		{
			//数据发送完成.
			break;
		}

		if (!bSendData)
		{
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExcep);
			FD_SET(skt, &fdWrite);
			FD_SET(skt, &fdExcep);

			stTimeOut.tv_sec = 0;
			stTimeOut.tv_usec = 10000;
			int iRet = select(-1/*skt + 1*/, NULL, &fdWrite, &fdExcep, &stTimeOut);
			if (iRet > 0)
			{
				iRet = FD_ISSET(skt, &fdExcep);
				if (iRet)
				{//error
					iError = -1;
					DBG_E;
					break;
				}
				//是否有数据
				iRet = FD_ISSET(skt, &fdWrite);
				if (iRet)
				{
					bSendData = true;
				}
			}
			else{
				// iret == 0 超时
				if (iRet == SOCKET_ERROR)
				{
					iError = -2;
					DBG_E;
					break;
				}
				if (skt == INVALID_SOCKET)
				{
					iError = -3;
					DBG_E;
					break;
				}
			}
		}
		else {
			iRet = ::send(skt, pBuff + iSendLen, iBuffLen - iSendLen, 0);
			bSendData = false;
			if (iRet > 0)
			{
				iSendLen += iRet;
			}
			if (iRet == SOCKET_ERROR)
			{
				int iErr = WSAGetLastError();
				if (iErr != WSAEWOULDBLOCK)
				{
					iError = -5;
					DBG_I("SOCKET err:%d", WSAGetLastError());
					break;
				}
			}
		}

		if ((!bSendData) && abs((int)(GetTickCount() - uiCurTimeOut)) >= (int)uiTimeOut)
		{
			//超时退出
			break;
		}
	} while (iSendLen < iBuffLen);
	if (nullptr != error){
		*error = iError;
	}
	return iSendLen;
}

unsigned int WriteFromUDP(SOCKET skt, const char* pBuff, int iBuffLen, struct sockaddr& to, int tolen, int* error /*= nullptr*/, unsigned int uiTimeOut /*= 5000*/)
{
	int iSendLen = 0, iError = 0, iRet = 0;

	if (nullptr == pBuff || iBuffLen <= 0)
	{
		DBG_E;
		return -1;
	}

	if (uiTimeOut <= 0)
	{
		iRet = ::sendto(skt, pBuff, iBuffLen, 0, (struct sockaddr*)&to, sizeof(sockaddr));
		iSendLen = iRet;
	}
	else{

		bool bSendData = true; //是否可以发送数据
		unsigned int uiCurTimeOut = GetTickCount();
		fd_set fdWrite, fdExcep;
		struct timeval stTimeOut;
		if (NULL != error)
		{
			*error = iError;
		}

		do
		{
			if (iBuffLen - iSendLen <= 0)
			{
				//数据发送完成.
				break;
			}

			if (!bSendData)
			{
				FD_ZERO(&fdWrite);
				FD_ZERO(&fdExcep);
				FD_SET(skt, &fdWrite);
				FD_SET(skt, &fdExcep);

				stTimeOut.tv_sec = 0;
				stTimeOut.tv_usec = 10000;
				int iRet = select(-1/*skt + 1*/, NULL, &fdWrite, &fdExcep, &stTimeOut);
				if (iRet > 0)
				{
					iRet = FD_ISSET(skt, &fdExcep);
					if (iRet)
					{//error
						iError = -1;
						DBG_E;
						break;
					}
					//是否有数据
					iRet = FD_ISSET(skt, &fdWrite);
					if (iRet)
					{
						bSendData = true;
					}
				}
				else{
					// iret == 0 超时
					if (iRet == SOCKET_ERROR)
					{
						iError = -2;
						DBG_E;
						break;
					}
					if (skt == INVALID_SOCKET)
					{
						iError = -3;
						DBG_E;
						break;
					}
				}
			}
			else {
				iRet = ::sendto(skt, pBuff + iSendLen, iBuffLen - iSendLen, 0, &to, (socklen_t)tolen);
				bSendData = false;
				if (iRet > 0)
				{
					iSendLen += iRet;
				}
				if (iRet == -1)
				{
					int iErr = WSAGetLastError();
					if (iErr != WSAEWOULDBLOCK)
					{
						iError = -4;
						DBG_I("SOCKET err:%d", WSAGetLastError());
						break;
					}
				}
			}
			if ((!bSendData) && abs((int)(GetTickCount() - uiCurTimeOut)) >= (int)uiTimeOut)
			{
				//超时退出
				break;
			}

		} while (iSendLen < iBuffLen);
	}

	if (nullptr != error){
		*error = iError;
	}
	return iSendLen;
}

CNet::CNet()
:m_nTransProType(TRANS_PROTOCOL_TYPE_NONE), m_ErrorCode(0), m_Skt(INVALID_SOCKET)
{
	InitSocket();
}
CNet::CNet(SYS_UTL::NET::TRANS_PROTOCOL_TYPE nType)
	: m_nTransProType(nType), m_ErrorCode(0), m_Skt(INVALID_SOCKET)
{
	InitSocket();
}
CNet::CNet(SOCKET skt, SYS_UTL::NET::TRANS_PROTOCOL_TYPE nType)
	: m_nTransProType(nType), m_ErrorCode(0), m_Skt(skt)
{

}

CNet::~CNet()
{

}

void CNet::Close()
{
	if (m_Skt != INVALID_SOCKET)
	{
		SOCKETS::closewrap(m_Skt);
		//shutdown(_sock, SD_BOTH);
		m_Skt = INVALID_SOCKET;
	}
}

bool CNet::IsValidSkt() const
{
	return m_Skt != INVALID_SOCKET;
}

bool CNet::IsError() const
{
	return (m_ErrorCode != 0) || (m_Skt == INVALID_SOCKET);
}

SYS_UTL::NET::TRANS_PROTOCOL_TYPE CNet::GetTransProtocol() const
{
	return m_nTransProType;
}

int CNet::SetSktOpt(int level, int optname, const char* optval, int optlen)
{
	return setsockopt(m_Skt, level, optname, optval, optlen);
}

int CNet::GetSktOpt(int level, int optname, char* optval, int *optlen)
{
	return getsockopt(m_Skt, level, optname, optval, optlen);
}

int CNet::GetPeerAddrInfo(struct sockaddr& stAddr)
{
	return -1;
}

void CNet::SetErrorCode(int iErrorCode, int line)
{
	m_ErrorCode = iErrorCode;
	if (m_ErrorCode != 0)
	{
		DBG_I("err:%d net.err:%d line:%d", m_ErrorCode, errno, line);
	}
}

int CNet::SetSktNoBlock()
{
	uint32_t argp;
	argp = 1;
	int err = ioctlsocket(m_Skt, FIONBIO, (u_long*)&argp);
	if (err != 0){
		SetErrorCode(SOCKET_ERROR, __LINE__);
	}
	return 0;
}

int CNet::SetTcpNoDelay()
{
	if (m_nTransProType == SYS_UTL::NET::TRANS_PROTOCOL_TYPE_TCP){
		int err = 0, v = static_cast<int>(true);
		if ((err = setsockopt(m_Skt, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&v), sizeof(v))) != 0){
			SetErrorCode(SOCKET_ERROR, __LINE__);
		}
		return err;
	}
	return 0;
}

int CNet::SetSktSendBufferSize(unsigned int uiSize)
{
	int err = setsockopt(m_Skt, SOL_SOCKET, SO_SNDBUF, (const char*)&uiSize, sizeof(int));
	if (err != 0){
		SetErrorCode(SOCKET_ERROR, __LINE__);
	}
	return err;
}

int CNet::SetSktRecvBufferSize(unsigned int uiSize)
{
	int err = setsockopt(m_Skt, SOL_SOCKET, SO_RCVBUF, (const char*)&uiSize, sizeof(int));
	if (err != 0){
		SetErrorCode(SOCKET_ERROR, __LINE__);
	}
	return err;
}

int CNet::SetSktCloseLinger(unsigned int uiTime)
{
	// 当连接中断时，需要延迟关闭(linger)以保证所有数据都被传输
	linger Linger;
	int err = 0;
	Linger.l_onoff = (uiTime > 0 ? 1 : 0);
	Linger.l_linger = uiTime;
	err = setsockopt(m_Skt, SOL_SOCKET, SO_LINGER, (char*)&Linger, sizeof(Linger));
	if (err != 0){
		SetErrorCode(SOCKET_ERROR, __LINE__);
	}
	return err;
}

int CNet::SetSktReuseAddr(bool bReuse)
{
	int err = 0, v = static_cast<int>(bReuse);
	if ((err = setsockopt(m_Skt, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&v), sizeof(v))) != 0){
		SetErrorCode(SOCKET_ERROR, __LINE__);
	}
	return err;
}

unsigned int CNet::Read(char* pBuff, int iBuffLen, const int iReadLen /*= 0*/, unsigned int uiTimeOut /*= 5000*/)
{
	int iError = -1, iErrorCode = 0;
	switch (m_nTransProType)
	{
	case SYS_UTL::NET::TRANS_PROTOCOL_TYPE_NONE:
		break;
	case SYS_UTL::NET::TRANS_PROTOCOL_TYPE_TCP:
		iError = ReadFromTcp(m_Skt, pBuff, iBuffLen, iReadLen, &iErrorCode, uiTimeOut);
		break;
	case SYS_UTL::NET::TRANS_PROTOCOL_TYPE_UDP:{
		struct sockaddr stAddr;
		iError = ReadFromUdp(m_Skt, pBuff, iBuffLen, stAddr, sizeof(struct sockaddr), &iErrorCode, uiTimeOut);
	}
		break;
	default:
		break;
	}
	if (iErrorCode != 0){
		SetErrorCode(iErrorCode, __LINE__);
	}
	return iError;
}

unsigned int CNet::ReadFromUDP(char* pBuff, int iBuffLen, struct sockaddr& from, int fromlen, unsigned int uiTimeOut /*= 5000*/)
{
	int iErrorCode = 0;
	int err = ReadFromUdp(m_Skt, pBuff, iBuffLen, from, fromlen, &iErrorCode, uiTimeOut);
	if (iErrorCode != 0){
		SetErrorCode(iErrorCode, __LINE__);
	}
	return err;
}

unsigned int CNet::Write(const char* pBuff, int iBuffLen, unsigned int uiTimeOut /*= 5000*/)
{
	int iErrorCode = 0;
	unsigned int len = WriteFromTcp(m_Skt, pBuff, iBuffLen, &iErrorCode, uiTimeOut);
	if (iErrorCode != 0){
		SetErrorCode(iErrorCode, __LINE__);
	}
	return len;
}

unsigned int CNet::WriteToUDP(const char* pBuff, int iBuffLen, struct sockaddr& to, int tolen, unsigned int uiTimeOut /*= 5000*/)
{
	int iErrorCode = 0;
	unsigned int len = WriteFromUDP(m_Skt, pBuff, iBuffLen, to, tolen, &iErrorCode, uiTimeOut);
	if (iErrorCode != 0){
		SetErrorCode(iErrorCode, __LINE__);
	}
	return len;
}


CNetClient::CNetClient() :CNet(){

}
CNetClient::CNetClient(SYS_UTL::NET::TRANS_PROTOCOL_TYPE nType)
	: CNet(nType)
{

}
CNetClient::CNetClient(SOCKET skt, SYS_UTL::NET::TRANS_PROTOCOL_TYPE nType)
	: CNet(skt, nType)
{

}
CNetClient::CNetClient(SOCKET skt, SYS_UTL::NET::TRANS_PROTOCOL_TYPE nType, InetAddress local, InetAddress peer)
	: CNet(skt, nType)
{
	m_local = local;
	m_peer = peer;
}
CNetClient::~CNetClient()
{

}

int CNetClient::ConnectSocket(TRANS_PROTOCOL_TYPE nType, const char* host, unsigned short port, int* error /*= nullptr*/, bool bConn /*= true*/)
{
	if (nType != m_nTransProType)
	{
		m_nTransProType = nType;
	}
	SetErrorCode(0, __LINE__);
	int iErrorCode = 0;
	SOCKET skt = SYS_UTL::NET::ConnectSocket(m_nTransProType, host, port, &iErrorCode, bConn);
	if (iErrorCode != 0)
	{
		DBG_E;
		SetErrorCode(iErrorCode, __LINE__);
		return -1;
	}
	InetAddress addr(host, port);
	m_peer = addr;
	m_Skt = skt;
	return 0;
}

void CNetClient::Attach(SOCKET skt)
{
	SetErrorCode(0, __LINE__);
	m_nTransProType = TRANS_PROTOCOL_TYPE_TCP;
	if (m_Skt != INVALID_SOCKET)
	{
		Close();
	}
	m_Skt = skt;
}

const InetAddress& CNetClient::GetPeerAddr()
{
	return m_peer;
}

CNetServer::CNetServer() :CNet(){

}
CNetServer::CNetServer(SYS_UTL::NET::TRANS_PROTOCOL_TYPE nType)
	: CNet(nType)
{

}

CNetServer::~CNetServer()
{

}

int CNetServer::OpenSocket(TRANS_PROTOCOL_TYPE nType, const char* host, unsigned short port, int* error /*= nullptr*/)
{
	if (nType != m_nTransProType)
	{
		m_nTransProType = nType;
	}
	SetErrorCode(0, __LINE__);
	int iErrorCode = 0;
	SOCKET skt = SYS_UTL::NET::OpenSocket(m_nTransProType, host, port, &iErrorCode);
	if (iErrorCode != 0)
	{
		SetErrorCode(iErrorCode, __LINE__);
		return -1;
	}
	m_Skt = skt;
	return 0;
}

bool CNetServer::Accept(CNetClient*& pClientIns)
{
	//int skt = ::accept(m_Skt, nullptr, nullptr);
	struct sockaddr_in6 addr;
	memZero(&addr, sizeof addr);
	SOCKET sockfd = SOCKETS::acceptOrDie(m_Skt, &addr);
	if (sockfd != INVALID_SOCKET)
	{
		InetAddress peerAddr;
		peerAddr.setSockAddrInet6(addr);
		pClientIns = new CNetClient(sockfd, TRANS_PROTOCOL_TYPE_TCP);
		if (pClientIns == nullptr)
		{
			return false;
		}
		DBG_I("新客户端[%s]连接", peerAddr.toIpPort().data());
		return true;
	}
	return false;
}

SOCKET CNetServer::Accept()
{
	return ::accept(m_Skt, (struct sockaddr*)nullptr, nullptr);
}
}
}