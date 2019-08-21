#include "../../include/log/Logging.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <time.h>
#include <fstream>

#include <windows.h>
#include "../InDef.h"
#include "../../include/api.h"
#include "../../include/ConfigFile.h"

std::atomic_flag lock = ATOMIC_FLAG_INIT;

enum LOG_MODE
{
	LOG_MODE_CONSOLE =1,
	LOG_MODE_DBGVIEW =2,
	LOG_MODE_FILE =4,
};

int gLogMode = 1;

/*
对应的颜色码表：
1. 0 = 黑色 8 = 灰色
1 = 蓝色 9 = 淡蓝色
2 = 绿色 10 = 淡绿色
3 = 浅绿色 11 = 淡浅绿色
4 = 红色 12 = 淡红色
5 = 紫色 13 = 淡紫色
6 = 黄色 14 = 淡黄色
7 = 白色 15 = 亮白色
*/
static void SetColor(unsigned short ForeColor = 4, unsigned short BackGroundColor = 0) 
//给参数默认值，使它//可以接受0/1/2个参数
{
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	if (NULL != hCon)
	{
		//SetConsoleTextAttribute(hCon, ForeColor | BackGroundColor);
		SetConsoleTextAttribute(hCon, ForeColor + BackGroundColor * 0x10);
	}
};

namespace SYS_UTL { namespace LOG {

__declspec(thread) char t_errnobuf[512];
__declspec(thread) char t_time[64];
__declspec(thread) time_t t_lastSecond;

const char* strerror_tl(int savedErrno)
{
	strerror_s(t_errnobuf, sizeof(t_errnobuf), savedErrno);
	return t_errnobuf;
}

Logger::LogLevel initLogLevel()
{
	int level = 0, tmp =0;
	char szPath[MAX_PATH];
	_snprintf_s(szPath, _TRUNCATE, "%s\\SysUtilities.ini", SYS_UTL::GetCurrentPath(ghInstance));
	SYS_UTL::ConfigFile::ReadInt(szPath, "LOG", "LEVEL", level, 0);
	SYS_UTL::ConfigFile::ReadInt(szPath, "LOG", "MODEL", tmp, 1);
	gLogMode = tmp;
	return (Logger::LogLevel)level;
	//if (::getenv("MUDUO_LOG_TRACE"))
	//	return Logger::TRACE;
	//else if (::getenv("MUDUO_LOG_DEBUG"))
	//	return Logger::DEBUG;
	//else
	//	return Logger::INFO;
}

Logger::LogLevel g_logLevel = initLogLevel();

const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
{
	"TRACE ",
	"DEBUG ",
	"INFO  ",
	"WARN  ",
	"ERROR ",
	"FATAL ",
};

// helper class for known string length at compile time
class T
{
public:
	T(const char* str, unsigned len)
		:str_(str),
		len_(len)
	{
		assert(strlen(str) == len_);
	}

	const char* str_;
	const unsigned len_;
};

inline LogStream& operator<<(LogStream& s, T v)
{
	s.append(v.str_, v.len_);
	return s;
}

inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v)
{
	s.append(v.data_, v.size_);
	return s;
}

void defaultOutput(const char* msg, int len)
{
#ifdef _WIN32
	if (!lock.test_and_set()){
		if (!::AllocConsole())
		{
			lock.clear();
			OutputDebugString("error");
		}
		else{
			FILE* pCout;
			freopen_s(&pCout, "CONOUT$", "w", stdout);//重定向输出流至控制台
		}
	}

#endif
	size_t n = fwrite(msg, 1, len, stdout);
	//FIXME check n
	(void)n;
}

void defaultFlush()
{
	fflush(stdout);
}

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;
//TimeZone g_logTimeZone;

}  // namespace LOG
}// namespace SYS_UTL

using namespace SYS_UTL::LOG;

Logger::Impl::Impl(LogLevel level, int savedErrno, const SourceFile& file, int line)
  : //time_(Timestamp::now()),
    stream_(),
    level_(level),
    line_(line),
    basename_(file)
{
	formatTime();
	//CurrentThread::tid();
	//stream_ << T(CurrentThread::tidString(), CurrentThread::tidStringLength());
	stream_ << T(LogLevelName[level], 6);
	if (savedErrno != 0)
	{
		stream_ << strerror_tl(savedErrno) << " (errno=" << savedErrno << ") ";
	}
}

void Logger::Impl::formatTime()
{
	//time_t seconds =0;
	//struct tm tm_time;

#if 1
	//::gmtime_s(&tm_time, &seconds); // FIXME TimeZone::fromUtcTime
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);
#else
	if (g_logTimeZone.valid())
	{
		tm_time = g_logTimeZone.toLocalTime(seconds);
	}
	else
	{
		::gmtime_s(&tm_time, &seconds); // FIXME TimeZone::fromUtcTime
	}
#endif

	//int len = _snprintf_s(t_time, _TRUNCATE, "%4d%02d%02d %02d:%02d:%02d",
	//	tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
	//	tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
	int len = _snprintf_s(t_time, _TRUNCATE, "%4d%02d%02d %02d:%02d:%02d",
		wtm.wYear, wtm.wMonth, wtm.wDay,
		wtm.wHour, wtm.wMinute, wtm.wSecond);
	assert(len == 17); (void)len;

	//Fmt us(".%06dZ ", seconds);
	//assert(us.length() == 9);
	stream_ << T(t_time, 17) <<" " /*<< T(us.data(), us.length())*/;
}

void Logger::Impl::finish()
{
	stream_ << " - " << basename_ << ':' << line_ << '\n';
}

Logger::Logger(SourceFile file, int line)
	: impl_(Logger::INFO, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
  : impl_(level, 0, file, line)
{
	impl_.stream_ << func << ' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level)
  : impl_(level, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, bool toAbort)
	: impl_((LogLevel)(toAbort ? Logger::FATAL : Logger::ERR), errno, file, line)
{
}

Logger::~Logger()
{
	impl_.finish();
	const LogStream::Buffer& buf(stream().buffer());
	if (gLogMode&LOG_MODE_CONSOLE)
	{
		if (impl_.level_ == Logger::WARN)
		{
			SetColor(6 | FOREGROUND_INTENSITY, 0);
		}
		else if (impl_.level_ >= Logger::ERR)
		{
			SetColor(FOREGROUND_RED | FOREGROUND_INTENSITY, 0);
		}
		g_output(buf.data(), buf.length());
		if (impl_.level_ >= Logger::WARN)
		{
			SetColor(7, 0);
		}
	}
	if (gLogMode&LOG_MODE_DBGVIEW)
	{
		stream().append("\0", 1);
		const char* lpInfo = buf.data();
		if (NULL != lpInfo)
		{
			OutputDebugString(lpInfo);
		}
	}
	if (gLogMode&LOG_MODE_FILE)
	{
		std::fstream fs;
		char szPath[MAX_PATH];
		_snprintf_s(szPath, _TRUNCATE, "%s\\SysUtilities.log", SYS_UTL::GetCurrentPath(ghInstance));
		fs.open(szPath, std::ios_base::in | std::ios_base::app);
		if (fs.is_open() && (!fs.bad())){
			fs << std::endl << std::string(buf.data(), buf.length());
		}
		fs.close();
	}
	if (impl_.level_ == Logger::FATAL)
	{
		g_flush();
		abort();
	}
}

void Logger::setLogLevel(Logger::LogLevel level)
{
	g_logLevel = level;
}

void Logger::setOutput(OutputFunc out)
{
	g_output = out;
}

void Logger::setFlush(FlushFunc flush)
{
	g_flush = flush;
}