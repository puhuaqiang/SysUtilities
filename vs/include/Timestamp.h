#ifndef __SYS_UTILITIES_TIMESTAMP_H__
#define __SYS_UTILITIES_TIMESTAMP_H__

#include "SysUtilities.h"
#include "CopyAble.h"

namespace SYS_UTL
{
	///
	/// 时间戳，以微秒表示。 这个类是不可变的。 建议按值传递它。
	/// src [muduo]
	///
	class SYS_UTL_CPPAPI CTimestamp : public SYS_UTL::CCopyAble
	{
	public:
		/// 构造无效的时间戳
		CTimestamp()
			: microSecondsSinceEpoch_(0)
		{
		}
		///
		/// Constucts a Timestamp at specific time
		///
		explicit CTimestamp(int64_t microSecondsSinceEpochArg)
			: microSecondsSinceEpoch_(microSecondsSinceEpochArg)
		{
		}

		void swap(CTimestamp& that)
		{
			std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
		}

		std::string toString() const;
		std::string toFormattedString(bool showMicroseconds = true) const;

		bool valid() const { return microSecondsSinceEpoch_ > 0; }

		int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
		time_t secondsSinceEpoch() const
		{
			return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
		}

		///
		/// Get time of now.
		///
		static CTimestamp now();
		static CTimestamp invalid()
		{
			return CTimestamp();
		}

		static const int kMicroSecondsPerSecond = 1000 * 1000;

	private:
		int64_t microSecondsSinceEpoch_;
	};

	inline bool operator<(CTimestamp lhs, CTimestamp rhs)
	{
		return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
	}

	inline bool operator==(CTimestamp lhs, CTimestamp rhs)
	{
		return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
	}

	///
	/// 获取两个时间戳的时间差，以秒为单位。
	///
	/// @param high, low
	/// @return (high-low) in seconds
	/// @c double has 52-bit precision, enough for one-microsecond
	inline double timeDifference(CTimestamp high, CTimestamp low)
	{
		int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
		return static_cast<double>(diff) / CTimestamp::kMicroSecondsPerSecond;
	}

	///
	/// 将N秒添加到给定的时间戳。
	///
	/// @return timestamp+seconds as Timestamp
	///
	inline CTimestamp addTime(CTimestamp timestamp, double seconds)
	{
		int64_t delta = static_cast<int64_t>(seconds * CTimestamp::kMicroSecondsPerSecond);
		return CTimestamp(timestamp.microSecondsSinceEpoch() + delta);
	}

	/**
	\brief 时间操作管理..
	*/
	class SYS_UTL_CPPAPI CTimeWrapper
	{
	public:
		typedef struct _TIME_INFO
		{
			short wYear;
			short wMonth;
			short wDay;
			short wHour;
			short wMinute;
			short wSecond;
		}TIME_INFO, *PTIME_INFO;
	public:
		CTimeWrapper();
		~CTimeWrapper();

		/**
		\brief 获取本地当前时间
		\return time_t
		*/
		static void GetNowTime(TIME_INFO&);
		/**
		\brief 将时间转换为自1970年1月1日以来持续时间的秒数，
		\return time_t
		*/
		static time_t Convert(short wYear, short wMonth, short wDay,
			short wHour, short wMinute, short wSecond);
		static time_t Convert(TIME_INFO&);
		static int Convert(time_t, TIME_INFO&);


	private:
		CTimeWrapper(const CTimeWrapper &);
		CTimeWrapper &operator=(const CTimeWrapper &);
	};

	/**
	\brief 时间戳..
	*/
	class SYS_UTL_CPPAPI CTimeStampWrapper
	{
		CTimeStampWrapper(const CTimeStampWrapper &ref);
		CTimeStampWrapper &operator=(const CTimeStampWrapper &ref);
	private:
		unsigned long long _uiNow;
		int _iTimeOut;
	public:
		CTimeStampWrapper();
		~CTimeStampWrapper();

		/**
		* @brief 初始化时间戳
		* @param iTimeOut 超时时间 毫秒
		* @return void
		*/
		void Init(int iTimeOut);

		/**
		* @brief 更新时间戳
		* @return void
		*/
		void Update();

		/**
		* @brief 检测本地时间戳是否已经到达超时时间.
		* @param update 是否更新时间戳
		* @return bool 如果达到超时时间,返回true,否则返回false
		*/
		bool Check(bool update = true);

		/**
		* @brief 检测本地时间戳是否已经到达超时时间.
		* @param iTimeOut 超时时间 毫秒
		* @return bool 如果达到超时时间,返回true,否则返回false
		*/
		bool Check(int iTimeOut);

		int Diff();

	private:
		void __Update();
	};
}

#endif	//__CRITSEC_H__
