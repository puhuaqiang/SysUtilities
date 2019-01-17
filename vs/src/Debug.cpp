#include "../SysUtilities/stdafx.h"
#include "../include/Debug.h"

#include <fstream>

namespace SYS_UTL
{
	SYS_UTL_CAPI void dbgview_info(char* flag, char* fmt, ...) 
	{
		va_list args;
		char sLog[1024];
		char sOut[1024];
		sLog[0] = '\0';
		sOut[0] = '\0';
		va_start(args, fmt);
		_vsnprintf_s(sOut, sizeof(sOut)-1, fmt, args);
		va_end(args);
		if (nullptr != flag && strlen(flag) > 0) {
			strncat_s(sLog, 1024, "[", min(1023 - strlen(sLog), 1));
			strncat_s(sLog, 1024, flag, min(1023 - strlen(sLog), strlen(flag)));
			strncat_s(sLog, 1024, "] ", min(1023 - strlen(sLog), 2));
		}
		strncat_s(sLog, 1024, sOut, min(1023 - strlen(sLog), strlen(sOut)));
		OutputDebugString(sLog);
	}

	void __debug_info(const char* flag, const char* toFile, const char* file, int line, const char* txt)
	{
		char sLog[1024];

		if ((NULL == txt) || strlen(txt) <= 0)
		{
			return;
		}
		sLog[0] = '\0';
		if (NULL != flag && strlen(flag) > 0)
		{
			strncat_s(sLog, _TRUNCATE, "[", min(1, sizeof(sLog)-strlen(sLog) - 1));
			strncat_s(sLog, _TRUNCATE, flag, min(strlen(flag), sizeof(sLog)-strlen(sLog) - 1));
			strncat_s(sLog, _TRUNCATE, "] ", min(2, sizeof(sLog)-strlen(sLog) - 1));
		}

		if ((NULL != toFile) && (strlen(toFile) > 0))
		{
			SYSTEMTIME st;
			char szTime[64];
			GetLocalTime(&st);
			_snprintf_s(szTime, _TRUNCATE, "%04d-%02d-%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
			strncat_s(sLog, _TRUNCATE, "[", min(1, sizeof(sLog)-strlen(sLog) - 1));
			strncat_s(sLog, _TRUNCATE, szTime, min(strlen(szTime), sizeof(sLog)-strlen(sLog) - 1));
			strncat_s(sLog, _TRUNCATE, "] ", min(2, sizeof(sLog)-strlen(sLog) - 1));
		}

		strncat_s(sLog, _TRUNCATE, txt, min(strlen(txt), sizeof(sLog)-strlen(sLog) - 1));

		if (NULL != file && strlen(file) > 0)
		{
			strncat_s(sLog, _TRUNCATE, ".[", min(2, sizeof(sLog)-strlen(sLog) - 1));
			strncat_s(sLog, _TRUNCATE, file, min(strlen(file), sizeof(sLog)-strlen(sLog) - 1));
			strncat_s(sLog, _TRUNCATE, "] ", min(2, sizeof(sLog)-strlen(sLog) - 1));
		}
		if (line >= 0) {
			char tmp[32];
			_snprintf_s(tmp, _TRUNCATE, ".[%d]", line);
			strncat_s(sLog, _TRUNCATE, tmp, min(strlen(tmp), sizeof(sLog)-strlen(sLog) - 1));
		}
		OutputDebugString(sLog);
		if ((NULL != toFile) && (strlen(toFile) > 0))
		{
			std::fstream fs;
			fs.open(toFile, std::ios_base::in | std::ios_base::app);
			if (fs.is_open() && (!fs.bad())){
				fs << std::endl << sLog;
			}
			fs.close();
		}
	}

	SYS_UTL_CAPI void dbgview_info2(const char* flag, const char* file, int line, const char* fmt, ...)
	{
#ifdef _WIN32
		va_list args;
		char sOut[1024];
		sOut[0] = '\0';
		va_start(args, fmt);
		_vsnprintf_s(sOut, sizeof(sOut)-1, fmt, args);
		va_end(args);
		__debug_info(flag, NULL, file, line, sOut);
#else
#endif
	}

	SYS_UTL_CAPI void dbgview_info3(const char* flag, const char* toFile, const char* file, int line, const char* fmt, ...) {
#ifdef _WIN32
		va_list args;
		char sOut[1024];
		sOut[0] = '\0';
		va_start(args, fmt);
		_vsnprintf_s(sOut, sizeof(sOut)-1, fmt, args);
		va_end(args);
		__debug_info(flag, toFile, file, line, sOut);
#else
#endif
	}
}