#ifndef CTM_COMMON_TIME_TOOLS_H__
#define CTM_COMMON_TIME_TOOLS_H__

#include <time.h>
#include <string>

namespace ctm
{
	using std::string;

	// 获取当前时间戳（秒）
	inline time_t Timestamp() { return time(NULL); }
	
	// 获取当前时间戳（毫秒）
	unsigned long long MilliTimestamp();

	// 获取当前时间的毫秒数
	unsigned int MilliSeconds();

	enum TimeFmt
	{
		TTIME_FMT_0 = 0,   // HH:mm:ss 
		TTIME_FMT_1 = 1,   // HH:mm
		TTIME_FMT_2 = 2,   // HHmmss
		TTIME_FMT_3 = 3,   // HHmm
	};

	enum DateFmt
	{
		TDATE_FMT_0 = 0,  // yyyy-MM-dd
		TDATE_FMT_1 = 1,  // yyyy/MM/dd
		TDATE_FMT_2 = 2,  // yyyyMMdd
		TDATE_FMT_3 = 3,  // yy-MM-dd
		TDATE_FMT_4 = 4,  // yy/MM/dd
		TDATE_FMT_5 = 5,  // yyMMdd
	};

	// 时间结构体转指定格式的日期和时间
	string TimeTm2FormatDate(const struct tm* st, int dateFmt = TDATE_FMT_0);
	string TimeTm2FormatTime(const struct tm* st, int timeFmt = TTIME_FMT_0);

	// 时间戳体转指定格式的日期和时间
	inline string Timestamp2FormatDate(time_t time, int dateFmt = TDATE_FMT_0)
	{
		return TimeTm2FormatDate(localtime(&time), dateFmt);
	}
	inline string Timestamp2FormatTime(time_t time, int timeFmt = TTIME_FMT_0)
	{
		return TimeTm2FormatTime(localtime(&time), timeFmt);
	}

	// 时间戳转指定格式的字符串时间
	string Timestamp2FormatDateTime(time_t time,
									int dateFmt = TDATE_FMT_0,
									int timeFmt = TTIME_FMT_0,
									const string &join = " ");

	// 日期和时间
	inline string DateTime(int dateFmt = TDATE_FMT_0, int timeFmt = TTIME_FMT_0)
	{
		return Timestamp2FormatDateTime(time(NULL), dateFmt, timeFmt);
	}
	// 日期
	inline string Date(int dateFmt = TDATE_FMT_0)
	{
		return Timestamp2FormatDate(time(NULL), dateFmt);
	}
	// 时间
	inline string Time(int timeFmt = TTIME_FMT_0)
	{
		return Timestamp2FormatTime(time(NULL), timeFmt);
	}

	// 指定时间是星期几[0, 6]
	int DayOfWeek(time_t time = time(NULL));

	// 指定时间是当年的第几周[00, 53]
	int WeekOfYear(time_t time = time(NULL));

	// 毫秒时间戳转换为日期时间
	string MilliTimestamp2DateTime(unsigned long long time);
	
	// 计时器
	class CClock
	{
	public:
		CClock(const string& tips = "clock") :
			m_tips(tips),
			m_begin(MilliTimestamp())
		{
		}
		
		~CClock()
		{
		}

		unsigned long RunTimes() 
		{
			return MilliTimestamp() - m_begin;
		}

		unsigned long long BeginTime() const
		{
			return m_begin;
		}

		string RunInfo() const;
		
	private:
		string m_tips;
		unsigned long long m_begin;
	};
};

#endif
