#ifndef __SYS_UTILITIES_LOG_LOGGING_H__
#define __SYS_UTILITIES_LOG_LOGGING_H__

#include "LogStream.h"

namespace SYS_UTL { namespace LOG {

class SYS_UTL_CPPAPI Logger
{
public:
	enum LogLevel
	{
		SYSUTL_TRACE,
		SYSUTL_DEBUG,
		SYSUTL_INFO,
		SYSUTL_WARN,
		SYSUTL_ERROR,
		SYSUTL_FATAL,
		NUM_LOG_LEVELS,
	};

	class SourceFile
	{
	public:
		template<int N>
		SourceFile(const char(&arr)[N])
			: data_(arr),
			size_(N - 1)
		{
			const char* slash = strrchr(data_, '/'); // builtin function
			if (slash)
			{
				data_ = slash + 1;
				size_ -= static_cast<int>(data_ - arr);
			}
		}

		explicit SourceFile(const char* filename)
			: data_(filename)
		{
			const char* slash = strrchr(filename, '/');
			if (slash)
			{
				data_ = slash + 1;
			}
			size_ = static_cast<int>(strlen(data_));
		}

		const char* data_;
		int size_;
	};

	Logger(SourceFile file, int line);
	Logger(SourceFile file, int line, LogLevel level);
	Logger(SourceFile file, int line, LogLevel level, const char* func);
	Logger(SourceFile file, int line, bool toAbort);
	~Logger();

	LogStream& stream() { return impl_.stream_; }

	static LogLevel logLevel();
	static void setLogLevel(LogLevel level);

	typedef void(*OutputFunc)(const char* msg, int len);
	typedef void(*FlushFunc)();
	static void setOutput(OutputFunc);
	static void setFlush(FlushFunc);

private:

	class Impl
	{
	public:
		typedef Logger::LogLevel LogLevel;
		Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
		void formatTime();
		void finish();

		//Timestamp time_;
		LogStream stream_;
		LogLevel level_;
		int line_;
		SourceFile basename_;
	};
	
	Impl impl_;

};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel()
{
	return g_logLevel;
}

#define LOG_TRACE if (SYS_UTL::LOG::Logger::logLevel() <= SYS_UTL::LOG::Logger::SYSUTL_TRACE) \
	SYS_UTL::LOG::Logger(__FILE__, __LINE__, SYS_UTL::LOG::Logger::SYSUTL_TRACE, __FUNCTION__ /*__func__*/).stream()
#define LOG_DEBUG if (SYS_UTL::LOG::Logger::logLevel() <= SYS_UTL::LOG::Logger::SYSUTL_DEBUG) \
	SYS_UTL::LOG::Logger(__FILE__, __LINE__, SYS_UTL::LOG::Logger::SYSUTL_DEBUG, __FUNCTION__).stream()
#define LOG_INFO if (SYS_UTL::LOG::Logger::logLevel() <= SYS_UTL::LOG::Logger::SYSUTL_INFO) \
	SYS_UTL::LOG::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN if (SYS_UTL::LOG::Logger::logLevel() <= SYS_UTL::LOG::Logger::SYSUTL_WARN) \
	SYS_UTL::LOG::Logger(__FILE__, __LINE__, SYS_UTL::LOG::Logger::SYSUTL_WARN, __FUNCTION__).stream()
#define LOG_ERROR if (SYS_UTL::LOG::Logger::logLevel() <= SYS_UTL::LOG::Logger::SYSUTL_ERROR) \
	SYS_UTL::LOG::Logger(__FILE__, __LINE__, SYS_UTL::LOG::Logger::SYSUTL_ERROR, __FUNCTION__).stream()
#define LOG_FATAL if (SYS_UTL::LOG::Logger::logLevel() <= SYS_UTL::LOG::Logger::SYSUTL_FATAL) \
	SYS_UTL::LOG::Logger(__FILE__, __LINE__, SYS_UTL::LOG::Logger::SYSUTL_FATAL, __FUNCTION__).stream()

//#define LOG_WARN SYS_UTL::LOG::Logger(__FILE__, __LINE__, SYS_UTL::LOG::Logger::SYSUTL_WARN).stream()
//#define LOG_ERROR SYS_UTL::LOG::Logger(__FILE__, __LINE__, SYS_UTL::LOG::Logger::SYSUTL_ERROR/*ERROR*/).stream()
//#define LOG_FATAL SYS_UTL::LOG::Logger(__FILE__, __LINE__, SYS_UTL::LOG::Logger::SYSUTL_FATAL).stream()
//#define LOG_SYSERR SYS_UTL::LOG::Logger(__FILE__, __LINE__, false).stream()
//#define LOG_SYSFATAL SYS_UTL::LOG::Logger(__FILE__, __LINE__, true).stream()

const char* strerror_tl(int savedErrno);

// Taken from glog/logging.h
//
// Check that the input is non NULL.  This very useful in constructor
// initializer lists.

#define CHECK_NOTNULL(val) \
  ::SYS_UTL::LOG::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

// A small helper for CHECK_NOTNULL().
template <typename T>
T* CheckNotNull(Logger::SourceFile file, int line, const char *names, T* ptr)
{
	if (ptr == NULL)
	{
		Logger(file, line, Logger::SYSUTL_FATAL).stream() << names;
	}
	return ptr;
}

} // namespace sys_utl
}// namespace log

#endif  // 
