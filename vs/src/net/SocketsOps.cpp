// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
#include "../../include/net/SocketsOps.h"
#include "../../include/SysUtilities.h"

#include "../../include/Types.h"
#include "Endian.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>  // snprintf
//#include <sys/socket.h>
//#include <sys/uio.h>  // readv
//#include <unistd.h>
#include "../../include/api.h"
#include "../InDef.h"

using namespace SYS_UTL;
using namespace SYS_UTL::NET;

namespace
{

typedef struct sockaddr SA;


#if VALGRIND || defined (NO_ACCEPT4)
void setNonBlockAndCloseOnExec(int sockfd)
{
  // non-block
  int flags = ::fcntl(sockfd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  int ret = ::fcntl(sockfd, F_SETFL, flags);
  // FIXME check

  // close-on-exec
  flags = ::fcntl(sockfd, F_GETFD, 0);
  flags |= FD_CLOEXEC;
  ret = ::fcntl(sockfd, F_SETFD, flags);
  // FIXME check

  (void)ret;
}
#endif

}  // namespace

const struct sockaddr* SOCKETS::sockaddr_cast(const struct sockaddr_in6* addr)
{
  return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

SYS_UTL_CAPI struct sockaddr* SOCKETS::sockaddr_cast(struct sockaddr_in6* addr)
{
  return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}

const struct sockaddr* SOCKETS::sockaddr_cast(const struct sockaddr_in* addr)
{
  return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

const struct sockaddr_in* SOCKETS::sockaddr_in_cast(const struct sockaddr* addr)
{
  return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}

const struct sockaddr_in6* SOCKETS::sockaddr_in6_cast(const struct sockaddr* addr)
{
  return static_cast<const struct sockaddr_in6*>(implicit_cast<const void*>(addr));
}

SYS_UTL_CAPI int SOCKETS::createNonblockingOrDie(short family)
{
	return -1;
}

SYS_UTL_CAPI void SOCKETS::bindOrDie(SOCKET sockfd, const struct sockaddr* addr)
{
	int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
	if (ret < 0)
	{
		DBG_I("sockets::bindOrDie");
		DBG_E;
		abort();
	}
}

SYS_UTL_CAPI void SOCKETS::listenOrDie(SOCKET sockfd)
{
	int ret = ::listen(sockfd, SOMAXCONN);
	if (ret < 0)
	{
		DBG_I("sockets::listenOrDie");
		DBG_E;
		abort();
	}
}

SYS_UTL_CAPI SOCKET SOCKETS::acceptOrDie(SOCKET sockfd, struct sockaddr_in6* addr)
{
	socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
#if VALGRIND || defined (NO_ACCEPT4)
	int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
	setNonBlockAndCloseOnExec(connfd);
#else
	SOCKET connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
#endif
	if (connfd == INVALID_SOCKET)
	{
		int savedErrno = WSAGetLastError();
		switch (savedErrno)
		{
			case WSAEINTR:
			case WSAEMFILE: // 在进入接受时队列是非空的，并且没有可用的描述符。?
				// expected errors
				break;
			case WSAEFAULT://The addrlen parameter is too small or addr is not a valid part of the user address space.
			case WSAEINVAL://在接受之前未调用listen函数。
			case WSAENOBUFS:
			case WSAENOTSOCK:
			case WSAEOPNOTSUPP://引用的套接字不是支持面向连接的服务的类型。
				// unexpected errors
				DBG_I("sockets::accept");
				DBG_E;
				abort();
				break;
			default:
				//DBG_I("sockets::accept");
				//DBG_E;
				//abort();
				break;
		}
	}
	else{
		//setNonBlockAndCloseOnExec(connfd);
		u_long argp;
		argp = 1;
		ioctlsocket(connfd, FIONBIO, (u_long*)&argp);//异步
		int on = 1;
		::setsockopt(connfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
		/// 禁止从容关闭
		linger _linger;
		_linger.l_onoff = 1;
		_linger.l_linger = 0;
		::setsockopt(connfd, SOL_SOCKET, SO_LINGER, (char *)&_linger, sizeof(_linger));
	}
	return connfd;
}

SYS_UTL_CAPI int SOCKETS::connectwrap(SOCKET sockfd, const struct sockaddr* addr)
{
  //return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
  return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr)));
}

SYS_UTL_CAPI int SOCKETS::readwrap(SOCKET sockfd, void *buf, size_t count)
{
	return ::recv(sockfd, (char*)buf, count, 0);
}

SYS_UTL_CAPI int SOCKETS::readvwrap(SOCKET sockfd, const struct iovec *iov, int iovcnt)
{
	DWORD  n = 0;//count of received
	DWORD flag = 0;
	return ::WSARecv(sockfd, (LPWSABUF)iov, iovcnt, &n, &flag, NULL, NULL);
}

SYS_UTL_CAPI int SOCKETS::writewrap(SOCKET sockfd, const void *buf, size_t count)
{
	return ::send(sockfd, (const char*)buf, count, 0);
}

SYS_UTL_CAPI void SOCKETS::closewrap(SOCKET sockfd)
{
	if (::closesocket(sockfd) != 0)
	{
		DBG_I("sockets::close");
		DBG_E;
	}
}

SYS_UTL_CAPI void SOCKETS::shutdownWrite(SOCKET sockfd)
{
	if (::shutdown(sockfd, SD_SEND) != 0)
	{
		DBG_I("sockets::shutdownWrite");
		DBG_E;
	}
}

SYS_UTL_CAPI void SOCKETS::toIpPort(char* buf, size_t size,
                       const struct sockaddr* addr)
{
	toIp(buf,size, addr);
	size_t end = ::strlen(buf);
	const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
	uint16_t port = SOCKETS::networkToHost16(addr4->sin_port);
	assert(size > end);
	sprintf_s(buf+end, size-end, ":%u", port);
}

SYS_UTL_CAPI void SOCKETS::toIp(char* buf, size_t size,
                   const struct sockaddr* addr)
{
	if (addr->sa_family == AF_INET)
	{
		assert(size >= 16/*INET_ADDRSTRLEN*/);
		const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
		::inet_ntop(AF_INET, (void*)&addr4->sin_addr, buf, size);
	}
	else if (addr->sa_family == AF_INET6)
	{
		assert(size >= 46/*INET6_ADDRSTRLEN*/);
		const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
		::inet_ntop(AF_INET6, (void*)&addr6->sin6_addr, buf, size);
	}

}

SYS_UTL_CAPI void SOCKETS::fromIpPort(const char* ip, uint16_t port,
                         struct sockaddr_in* addr)
{
	addr->sin_family = AF_INET;
	addr->sin_port = hostToNetwork16(port);
	if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
	{
		DBG_I("sockets::fromIpPort");
		DBG_E;		
	}
}

SYS_UTL_CAPI void SOCKETS::fromIpPortv6(const char* ip, uint16_t port,
                         struct sockaddr_in6* addr)
{
	addr->sin6_family = AF_INET6;
	addr->sin6_port = hostToNetwork16(port);
	if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0)
	{
		DBG_I("sockets::fromIpPortv6");
		DBG_E;
	}
}

SYS_UTL_CAPI int SOCKETS::getSocketError(int sockfd)
{
	int optval;
	socklen_t optlen = static_cast<socklen_t>(sizeof optval);

	if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*)&optval, &optlen) < 0)
	{
		return errno;
	}
	else
	{
		return optval;
	}
}

SYS_UTL_CAPI struct sockaddr_in6 SOCKETS::getLocalAddr(SOCKET sockfd)
{
	struct sockaddr_in6 localaddr;
	memZero(&localaddr, sizeof localaddr);
	socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
	if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0)
	{
		DBG_I("sockets::getLocalAddr");
		DBG_E;		
 	}
	return localaddr;
}

SYS_UTL_CAPI struct sockaddr_in6 SOCKETS::getPeerAddr(SOCKET sockfd)
{
	struct sockaddr_in6 peeraddr;
	memZero(&peeraddr, sizeof peeraddr);
	socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
	if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0)
	{
		DBG_I("sockets::getPeerAddr");
		DBG_E;		
	}
	return peeraddr;
}

SYS_UTL_CAPI bool SOCKETS::isSelfConnect(SOCKET sockfd)
{
	struct sockaddr_in6 localaddr = getLocalAddr(sockfd);
	struct sockaddr_in6 peeraddr = getPeerAddr(sockfd);
	if (localaddr.sin6_family == AF_INET)
	{
		const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localaddr);
		const struct sockaddr_in* raddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr);
		return laddr4->sin_port == raddr4->sin_port
		    && laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
	}
	else if (localaddr.sin6_family == AF_INET6)
	{
		return localaddr.sin6_port == peeraddr.sin6_port
			&& memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof localaddr.sin6_addr) == 0;
	}
	else
	{
	  return false;
	}
}

SYS_UTL_CAPI int SYS_UTL::NET::SOCKETS::setReuseAddr(SOCKET sockfd, bool bReuse)
{
	int err = 0, v = static_cast<int>(bReuse);
	if ((err = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&v), sizeof(v))) != 0){
	}
	return err;
}

