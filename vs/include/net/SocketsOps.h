// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef __SYS_UTILITIES_NET_SOCKET_OPS_H__
#define __SYS_UTILITIES_NET_SOCKET_OPS_H__

#include "InetAddress.h"
#include "../SysUtilities.h"
namespace SYS_UTL
{
namespace NET
{
namespace SOCKETS
{

///
/// Creates a non-blocking socket file descriptor,
/// abort if any error.
SYS_UTL_CAPI int createNonblockingOrDie(short family);

SYS_UTL_CAPI int  connectwrap(SOCKET sockfd, const struct sockaddr* addr);
SYS_UTL_CAPI void bindOrDie(SOCKET sockfd, const struct sockaddr* addr);
SYS_UTL_CAPI void listenOrDie(SOCKET sockfd);
SYS_UTL_CAPI SOCKET acceptOrDie(SOCKET sockfd, struct sockaddr_in6* addr);
SYS_UTL_CAPI int readwrap(SOCKET sockfd, void *buf, size_t count);
SYS_UTL_CAPI int readvwrap(SOCKET sockfd, const struct iovec *iov, int iovcnt);
SYS_UTL_CAPI int writewrap(SOCKET sockfd, const void *buf, size_t count);
SYS_UTL_CAPI void closewrap(SOCKET sockfd);
SYS_UTL_CAPI void shutdownWrite(SOCKET sockfd);

SYS_UTL_CAPI void toIpPort(char* buf, size_t size,
              const struct sockaddr* addr);
SYS_UTL_CAPI void toIp(char* buf, size_t size,
          const struct sockaddr* addr);

SYS_UTL_CAPI void fromIpPort(const char* ip, uint16_t port,
                struct sockaddr_in* addr);
SYS_UTL_CAPI void fromIpPortv6(const char* ip, uint16_t port,
                struct sockaddr_in6* addr);

SYS_UTL_CAPI int getSocketError(int sockfd);

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
SYS_UTL_CAPI struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr);
const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);
const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr);

SYS_UTL_CAPI struct sockaddr_in6 getLocalAddr(SOCKET sockfd);
SYS_UTL_CAPI struct sockaddr_in6 getPeerAddr(SOCKET sockfd);
SYS_UTL_CAPI bool isSelfConnect(SOCKET sockfd);

}  // namespace sockets
}  // namespace net
}  // namespace 

#endif  // 
