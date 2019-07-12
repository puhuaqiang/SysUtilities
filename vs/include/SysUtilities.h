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

#include <atomic>
#include <functional>
#include <map>
#include <list>
#include <string>
#include <stdint.h>
#include <windows.h>
#include <assert.h>

#include "StringPiece.h"
#pragma warning(disable: 4251)

#include "Debug.h"
#ifndef DBG_I
#define DBG_I(...) SYS_UTL::dbgview_info2("SysUtility", __FILE__, __LINE__, __VA_ARGS__);
#endif
#ifndef DBG_E
#define DBG_E DBG_I("ERROR");
#endif
#ifndef DBG_F
#define DBG_F DBG_I("FATAL ERROR");
#endif

#endif