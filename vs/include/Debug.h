#ifndef __SYS_UTILITIES_DEBUG_H__
#define __SYS_UTILITIES_DEBUG_H__

namespace SYS_UTL
{
	/**
	* \brief ��ӡ������Ϣ.
	*	Windows ��ӡ��ϵͳ������Ϣ��.��ͨ�� dbgview.exe���߲鿴.
	* \param flag ģ���ʶ�ַ���..eg. [xxx] log...
	* \param file ģ��Դ���ļ���
	* \param line ģ��Դ������к�
	* \param toFile �Ƿ�д��־�ļ�,��־�ļ�ȫ·��. NULL��ʾ��д��־�ļ�.
	*/
	SYS_UTL_CAPI void dbgview_info(char* flag, char*, ...);
	SYS_UTL_CAPI void dbgview_info2(const char* flag, const char* file, int line, const char* fmt, ...);
	SYS_UTL_CAPI void dbgview_info3(const char* flag, const char* toFile, const char* file, int line, const char* fmt, ...);
}

#endif