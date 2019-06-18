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
}

#endif	//__CRITSEC_H__
