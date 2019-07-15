#ifndef __SYS_UTILITIES_TIMESTAMP_H__
#define __SYS_UTILITIES_TIMESTAMP_H__

#include "SysUtilities.h"
#include "CopyAble.h"

namespace SYS_UTL
{
	///
	/// ʱ�������΢���ʾ�� ������ǲ��ɱ�ġ� ���鰴ֵ��������
	/// src [muduo]
	///
	class SYS_UTL_CPPAPI CTimestamp : public SYS_UTL::CCopyAble
	{
	public:
		/// ������Ч��ʱ���
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
	/// ��ȡ����ʱ�����ʱ������Ϊ��λ��
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
	/// ��N����ӵ�������ʱ�����
	///
	/// @return timestamp+seconds as Timestamp
	///
	inline CTimestamp addTime(CTimestamp timestamp, double seconds)
	{
		int64_t delta = static_cast<int64_t>(seconds * CTimestamp::kMicroSecondsPerSecond);
		return CTimestamp(timestamp.microSecondsSinceEpoch() + delta);
	}

	/**
	\brief ʱ���������..
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
		\brief ��ȡ���ص�ǰʱ��
		\return time_t
		*/
		static void GetNowTime(TIME_INFO&);
		/**
		\brief ��ʱ��ת��Ϊ��1970��1��1����������ʱ���������
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
	\brief ʱ���..
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
		* @brief ��ʼ��ʱ���
		* @param iTimeOut ��ʱʱ�� ����
		* @return void
		*/
		void Init(int iTimeOut);

		/**
		* @brief ����ʱ���
		* @return void
		*/
		void Update();

		/**
		* @brief ��Ȿ��ʱ����Ƿ��Ѿ����ﳬʱʱ��.
		* @param update �Ƿ����ʱ���
		* @return bool ����ﵽ��ʱʱ��,����true,���򷵻�false
		*/
		bool Check(bool update = true);

		/**
		* @brief ��Ȿ��ʱ����Ƿ��Ѿ����ﳬʱʱ��.
		* @param iTimeOut ��ʱʱ�� ����
		* @return bool ����ﵽ��ʱʱ��,����true,���򷵻�false
		*/
		bool Check(int iTimeOut);

		int Diff();

	private:
		void __Update();
	};
}

#endif	//__CRITSEC_H__
