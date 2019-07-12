// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef __SYS_UTILITIES_NET_ENDIAN_H__
#define __SYS_UTILITIES_NET_ENDIAN_H__

#include <stdint.h>
/*#include <endian.h>*/
#include <ws2ipdef.h>
#include <WS2tcpip.h>

namespace SYS_UTL
{
namespace NET
{
namespace SOCKETS
{

// the inline assembler code makes type blur,
// so we disable warnings for a while.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"
inline uint64_t hostToNetwork64(uint64_t host64)
{
	return htonll(host64);
}

inline uint32_t hostToNetwork32(uint32_t host32)
{
	return htonl(host32);
}

inline uint16_t hostToNetwork16(uint16_t host16)
{
	return htons(host16);
}

inline uint64_t networkToHost64(uint64_t net64)
{
	return ntohll(net64);
}

inline uint32_t networkToHost32(uint32_t net32)
{
	return ntohl(net32);
}

inline uint16_t networkToHost16(uint16_t net16)
{
	return ntohs(net16);
}

#pragma GCC diagnostic pop

}  // namespace sockets
}  // namespace net
}  // namespace 

#endif  // MUDUO_NET_ENDIAN_H
