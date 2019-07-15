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
		
#if 1
		struct tm tm_time;
		//gmtime_s(&tm_time, &seconds);// FIXME: localtime_r ?
		localtime_s(&tm_time, &seconds);
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

	CTimeWrapper::CTimeWrapper()
	{
	}
	CTimeWrapper::~CTimeWrapper()
	{

	}

	time_t CTimeWrapper::Convert(short wYear, short wMonth, short wDay, short wHour, short wMinute, short wSecond)
	{
		short wM = 0, wY = 0;
		wY = (wYear == 0 ? 0 : (wYear - 1900));
		wM = (wMonth == 0 ? 0 : (wMonth - 1));
		struct tm gm = { wSecond, wMinute, wHour, wDay, wM, wY, 0, 0, 0 };
		return mktime(&gm);
	}

	time_t CTimeWrapper::Convert(CTimeWrapper::TIME_INFO& v)
	{
		return Convert(v.wYear, v.wMonth, v.wDay, v.wHour, v.wMinute, v.wSecond);
	}

	int CTimeWrapper::Convert(time_t v, TIME_INFO& info)
	{
		tm ov;
		int err = localtime_s(&ov, &v);
		if (err == 0)
		{
			info.wYear = ov.tm_year + 1900;
			info.wMonth = ov.tm_mon + 1;
			info.wDay = ov.tm_mday;
			info.wHour = ov.tm_hour;
			info.wMinute = ov.tm_min;
			info.wSecond = ov.tm_sec;
		}
		return err;
	}

	void CTimeWrapper::GetNowTime(TIME_INFO& time) {
#ifdef _WIN32
		SYSTEMTIME st;
		GetLocalTime(&st);
		time.wYear = st.wYear;
		time.wMonth = st.wMonth;
		time.wDay = st.wDay;
		time.wHour = st.wHour;
		time.wMinute = st.wMinute;
		time.wSecond = st.wSecond;
#else
		static int gZone = 28800;
		static int gDayLightTime = 0;

		struct tm*	p;
		time_t timep;

		time(&timep);

		timep += gZone;
		timep += gDayLightTime;

		p = localtime(&timep);
		if (p != NULL){
			time.wYear = p->tm_year + 1900;
			time.wMonth = p->tm_mon + 1;
			time.wDay = p->tm_mday;
			//time.wDayOfWeek = p->tm_wday;
			time.wHour = p->tm_hour;
			time.wMinute = p->tm_min;
			time.wSecond = p->tm_sec;
			//time.wMilliseconds = 0;
		}
#endif
	}

	CTimeStampWrapper::CTimeStampWrapper()
	{
		_uiNow = 0;
		_iTimeOut = 0;
	}
	CTimeStampWrapper::~CTimeStampWrapper() {}

	void CTimeStampWrapper::Init(int iTimeOut)
	{
		__Update();
		_iTimeOut = iTimeOut;
	}

	void CTimeStampWrapper::Update()
	{
		__Update();
	}

	bool CTimeStampWrapper::Check(bool update /*= true*/)
	{
		if (abs((int)(GetTickCount64() - _uiNow)) >= _iTimeOut)
		{
			if (update) {
				__Update();
			}
			return true;
		}
		return false;
	}

	bool CTimeStampWrapper::Check(int iTimeOut)
	{
		if (abs((int)(GetTickCount64() - _uiNow)) >= iTimeOut)
		{
			return true;
		}
		return false;
	}

	int CTimeStampWrapper::Diff()
	{
		return (int)(GetTickCount64() - _uiNow);
	}

	void CTimeStampWrapper::__Update()
	{
		_uiNow = GetTickCount64();
	}
}
