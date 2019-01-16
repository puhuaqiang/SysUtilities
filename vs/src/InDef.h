#ifndef __SYS_UTILITIES_INDEF__
#define __SYS_UTILITIES_INDEF__

#if 1
#define DBG_INFO(...) dbgview_info2("SysUtilties",__FILE__,__LINE__,__VA_ARGS__);
#define DBG_ERROR DBG_INFO("ERROR")
#else
#define DBG_INFO(...)
#define DBG_ERROR
#endif

#endif