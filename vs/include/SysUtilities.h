#ifndef __SYS_UTILITIES_H__
#define __SYS_UTILITIES_H__

#ifdef SYS_UTL_EXPORTS
#define SYS_UTL_CAPI extern "C" __declspec(dllexport)
#else
#define SYS_UTL_CAPI extern "C" __declspec(dllimport)	
#endif

#ifdef SYS_UTL_EXPORTS
#define SYS_UTL_CPPAPI __declspec(dllexport)
#else
#define SYS_UTL_CPPAPI __declspec(dllimport)	
#endif

/// Four-byte alignment
#define PAD_SIZE(s) (((s)+3)&~3)

#include <functional>
#include <list>
#pragma warning(disable: 4251)
#endif