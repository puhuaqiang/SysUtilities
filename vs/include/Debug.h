#ifndef __SYS_UTILITIES_DEBUG_H__
#define __SYS_UTILITIES_DEBUG_H__

namespace SYS_UTL
{
	/**
	* \brief 打印调试信息.
	*	Windows 打印在系统调试信息中.可通过 dbgview.exe工具查看.
	* \param flag 模块标识字符串..eg. [xxx] log...
	* \param file 模块源码文件名
	* \param line 模块源码代码行号
	* \param toFile 是否写日志文件,日志文件全路径. NULL表示不写日志文件.
	*/
	SYS_UTL_CAPI void dbgview_info(char* flag, char*, ...);
	SYS_UTL_CAPI void dbgview_info2(const char* flag, const char* file, int line, const char* fmt, ...);
	SYS_UTL_CAPI void dbgview_info3(const char* flag, const char* toFile, const char* file, int line, const char* fmt, ...);
}

#endif