#include "../SysUtilities/stdafx.h"
#include "../include/Timestamp.h"

#include <time.h>

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>
#include <assert.h>

#include "InDef.h"

namespace SYS_UTL
{
	/**
	* #include <inttypes.h>
	* printf("%" PRId64 "\n", value);
	* // 相当于64位的：
	* printf("%" "ld" "\n", value);
	* // 或32位的：
	* printf("%" "lld" "\n", value);
	*/

	//static_assert(sizeof(Timestamp) == sizeof(int64_t),"Timestamp is same size as int64_t");

	std::string CTimestamp::toString() const
	{
		char buf[32] = { 0 };
		int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
		int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
		_snprintf_s(buf, _TRUNCATE, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
		return buf;
	}

	std::string CTimestamp::toFormattedString(bool showMicroseconds) const
	{
		char buf[64] = { 0 };
		time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
		
		DBG_INFO("toFormattedString...%d", seconds);
#if 1
		struct tm tm_time;
		gmtime_s(&tm_time, &seconds);// FIXME: localtime_r ?
#else
		struct tm tm_time;
		SYSTEMTIME wtm;
		GetLocalTime(&wtm);
		tm_time.tm_year = wtm.wYear - 1900;
		tm_time.tm_mon = wtm.wMonth - 1;
		tm_time.tm_mday = wtm.wDay;
		tm_time.tm_hour = wtm.wHour;
		tm_time.tm_min = wtm.wMinute;
		tm_time.tm_sec = wtm.wSecond;
		tm_time.tm_isdst = -1;
#endif
		if (showMicroseconds)
		{
			int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
			_snprintf_s(buf, _TRUNCATE, "%4d%02d%02d %02d:%02d:%02d.%06d",
				tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
				tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
				microseconds);
		}
		else
		{
			_snprintf_s(buf, _TRUNCATE, "%4d%02d%02d %02d:%02d:%02d",
				tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
				tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
		}
		return buf;
	}

	CTimestamp CTimestamp::now()
	{
		time_t clock;
		struct tm tm;
		SYSTEMTIME wtm;
		GetLocalTime(&wtm);
		tm.tm_year = wtm.wYear - 1900;
		tm.tm_mon = wtm.wMonth - 1;
		tm.tm_mday = wtm.wDay;
		tm.tm_hour = wtm.wHour;
		tm.tm_min = wtm.wMinute;
		tm.tm_sec = wtm.wSecond;
		tm.tm_isdst = -1;
		clock = mktime(&tm);
		return CTimestamp(clock * kMicroSecondsPerSecond + wtm.wMilliseconds * 1000);
	}
}
