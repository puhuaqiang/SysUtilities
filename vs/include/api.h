#ifndef __SYS_UTILITIES_API_H__
#define __SYS_UTILITIES_API_H__

typedef struct _SYSUTL_ONCE
{
	unsigned char ran;
	HANDLE event;
}SYSUTL_ONCE, *LPSYSUTL_ONCE;

/**
* \brief 功能函数
*/
namespace SYS_UTL
{

	/**
	* 多个线程可以尝试以一个给定的guard与一个函数指针
	* 这个函数会被调用一次,只有一次:
	*/
	SYS_UTL_CAPI void Once(LPSYSUTL_ONCE guard, void(*callback)(void));
	/*
	* 随机数 c++11
	*/
	SYS_UTL_CAPI int Random(int start, int end);
}

#endif