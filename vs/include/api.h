#ifndef __SYS_UTILITIES_API_H__
#define __SYS_UTILITIES_API_H__

typedef struct _SYSUTL_ONCE
{
	unsigned char ran;
	HANDLE event;
}SYSUTL_ONCE, *LPSYSUTL_ONCE;

/**
* \brief ���ܺ���
*/
namespace SYS_UTL
{

	/**
	* ����߳̿��Գ�����һ��������guard��һ������ָ��
	* ��������ᱻ����һ��,ֻ��һ��:
	*/
	SYS_UTL_CAPI void Once(LPSYSUTL_ONCE guard, void(*callback)(void));
	/*
	* ����� c++11
	*/
	SYS_UTL_CAPI int Random(int start, int end);
}

#endif