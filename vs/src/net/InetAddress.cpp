// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "../../include/net/InetAddress.h"
#include "Endian.h"
#include "../../include/net/SocketsOps.h"
#include "../InDef.h"
// #include <netdb.h>
// #include <netinet/in.h>

// INADDR_ANY use (type)value casting.
#pragma GCC diagnostic ignored "-Wold-style-cast"
static const unsigned int kInaddrAny = INADDR_ANY;
static const unsigned int kInaddrLoopback = INADDR_LOOPBACK;
#pragma GCC diagnostic error "-Wold-style-cast"

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

//     struct sockaddr_in6 {
//         sa_family_t     sin6_family;   /* address family: AF_INET6 */
//         uint16_t        sin6_port;     /* port in network byte order */
//         uint32_t        sin6_flowinfo; /* IPv6 flow information */
//         struct in6_addr sin6_addr;     /* IPv6 address */
//         uint32_t        sin6_scope_id; /* IPv6 scope-id */
//     };

/*static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6),
              "InetAddress is same size as sockaddr_in6");
static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin_family offset 0");
static_assert(offsetof(sockaddr_in6, sin6_family) == 0, "sin6_family offset 0");
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset 2");
static_assert(offsetof(sockaddr_in6, sin6_port) == 2, "sin6_port offset 2");*/

namespace SYS_UTL{namespace NET{
InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6)
{
	//static_assert(offsetof(InetAddress, addr6_) == 0, "addr6_ offset 0");
	//static_assert(offsetof(InetAddress, addr_) == 0, "addr_ offset 0");
	if (ipv6)
	{
		memZero(&addr6_, sizeof addr6_);
		addr6_.sin6_family = AF_INET6;
		in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
		addr6_.sin6_addr = ip;
		addr6_.sin6_port = SOCKETS::hostToNetwork16(port);
	}
	else
	{
		memZero(&addr_, sizeof addr_);
		addr_.sin_family = AF_INET;
		unsigned int ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
		addr_.sin_addr.s_addr = SOCKETS::hostToNetwork32(ip);
		addr_.sin_port = SOCKETS::hostToNetwork16(port);
	}
}

InetAddress::InetAddress(StringArg ip, uint16_t port, bool ipv6)
{
	if (ipv6)
	{
		memZero(&addr6_, sizeof addr6_);
		SOCKETS::fromIpPortv6(ip.c_str(), port, &addr6_);
	}
	else
	{
		memZero(&addr_, sizeof addr_);
		SOCKETS::fromIpPort(ip.c_str(), port, &addr_);
		DBG_I("ip:%s port:%d..", ip.c_str(), port);
	}
}

string InetAddress::toIpPort() const
{
	char buf[64] = "";
	SOCKETS::toIpPort(buf, sizeof buf, getSockAddr());
	return buf;
}

string InetAddress::toIp() const
{
	char buf[64] = "";
	SOCKETS::toIp(buf, sizeof buf, getSockAddr());
	return buf;
}

uint32_t InetAddress::ipNetEndian() const
{
	assert(family() == AF_INET);
	return addr_.sin_addr.s_addr;
}

uint16_t InetAddress::toPort() const
{
	return SOCKETS::networkToHost16(portNetEndian());
}

/*static __thread char t_resolveBuffer[64 * 1024];*/
static __declspec(thread) char t_resolveBuffer[64 * 1024];

bool InetAddress::resolve(StringArg hostname, InetAddress* out)
{
	assert(out != NULL);
	struct hostent hent;
	struct hostent* he = NULL;
	int herrno = 0;
	memZero(&hent, sizeof(hent));

	struct addrinfo hints;
	struct addrinfo *res, *cur;
	struct sockaddr_in *addr;

	memset(&hints, 0, sizeof(struct addrinfo));
	PADDRINFOT ai = nullptr;
	int const family = /*ipv6 ? AF_INET6 :*/ AF_INET;
	int const socket_type = SOCK_STREAM;
	int const protocol = 0/*IPPROTO_TCP*/;

	hints.ai_family = family;
	hints.ai_socktype = socket_type;
	hints.ai_protocol = protocol;
	hints.ai_flags = AI_PASSIVE;
	int ret = getaddrinfo(hostname.c_str(), NULL, &hints, &res);
	if (-1 != ret)
	{
		for (cur = res; cur != NULL; cur = cur->ai_next) {
			addr = (struct sockaddr_in *)cur->ai_addr;
			out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
		}
		return true;
	}
	else
	{
		DBG_I("InetAddress::resolve");
		return false;
	}
}

void InetAddress::setScopeId(uint32_t scope_id)
{
	if (family() == AF_INET6)
	{
		addr6_.sin6_scope_id = scope_id;
	}
}
}
}
