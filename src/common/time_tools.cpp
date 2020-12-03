#include "time_tools.h"
#include "com.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#endif

namespace ctm
{
	unsigned long MilliTimestamp()
	{
#ifdef WIN32
		SYSTEMTIME sys_time;
		GetLocalTime(&sys_time);
		return (time(NULL) * 1000 + sys_time.wMilliseconds);
#else
		struct timeval val = {0};
		gettimeofday(&val, NULL);
		return (val.tv_sec * 1000 + val.tv_usec / 1000);
#endif
		return 0;
	}

	unsigned int MilliSeconds()
	{
#ifdef WIN32
		SYSTEMTIME sys_time;
		GetLocalTime(&sys_time);
		return sys_time.wMilliseconds;
#else
		struct timeval val = { 0 };
		gettimeofday(&val, NULL);
		return (val.tv_usec / 1000);
#endif
		return 0;
	}

	string TimeTm2FormatDate(const struct tm* st, int dateFmt)
	{
		char buf[32] = {0};
		switch (dateFmt)
		{
		case TDATE_FMT_0:
			strftime(buf, sizeof(buf), "%Y-%m-%d", st);
			break;
		case TDATE_FMT_1:
			strftime(buf, sizeof(buf), "%Y/%m/%d", st);
			break;
		case TDATE_FMT_2:
			strftime(buf, sizeof(buf), "%Y%m%d", st);
			break;
		case TDATE_FMT_3:
			strftime(buf, sizeof(buf), "%y-%m-%d", st);
			break;
		case TDATE_FMT_4:
			strftime(buf, sizeof(buf), "%y/%m/%d", st);
			break;
		case TDATE_FMT_5:
			strftime(buf, sizeof(buf), "%y%m%d", st);
			break;
		default:
			strftime(buf, sizeof(buf), "%Y-%m-%d", st);
			break;
		}

		return std::string(buf);
	}

	string TimeTm2FormatTime(const struct tm* st, int timeFmt)
	{
		char buf[32] = {0};
		switch (timeFmt)
		{
		case TTIME_FMT_0:
			strftime(buf, sizeof(buf), "%H:%M:%S", st);
			break;
		case TTIME_FMT_1:
			strftime(buf, sizeof(buf), "%H:%M", st);
			break;
		case TTIME_FMT_2:
			strftime(buf, sizeof(buf), "%H%M%S", st);
			break;
		case TTIME_FMT_3:
			strftime(buf, sizeof(buf), "%H%M", st);
			break;
		default:
			strftime(buf, sizeof(buf), "%H:%M:%S", st);
			break;
		}

		return std::string(buf);
	}

	string Timestamp2FormatDateTime(time_t time, int dateFmt, int timeFmt, const string& join)
	{
		struct tm* st = localtime(&time);
		return TimeTm2FormatDate(st, dateFmt) + join + TimeTm2FormatTime(st, timeFmt);
	}

	int DayOfWeek(time_t time)
	{
		char buf[4] = {0};
		struct tm* st = localtime((const time_t *)&time);
		strftime(buf, sizeof(buf), "%w", st);

		return S2I(buf);
	}

	int WeekOfYear(time_t time)
	{
		char buf[4] = {0};
		struct tm* st = localtime((const time_t *)&time);
		strftime(buf, sizeof(buf), "%W", st);

		return S2I(buf);
	}

	int Timezone()
	{
		time_t t = time(NULL);
		char buf[16] = {0};
		struct tm* st = localtime((const time_t *)&t);
		strftime(buf, sizeof(buf), "%z", st);
		return S2I(buf) / 100;
	}

	time_t TodayBeginTime(time_t time)
	{
		return time - ((time + Timezone() * 3600) % (24 * 3600));
	}

	time_t TodayEndTime(time_t time)
	{
		return TodayBeginTime(time) +  24 * 3600 - 1;
	}

	time_t NextDayBeginTime(time_t time, int day)
	{
		return TodayBeginTime(time) +  day * 24 * 3600;
	}

	time_t NextDayEndTime(time_t time, int day)
	{
		return NextDayBeginTime(time, day) +  24 * 3600 - 1;
	}

	string MilliTimestamp2DateTime(unsigned long time)
	{
		char buf[64] = {0};
		snprintf(buf, sizeof(buf), "%s.%03u", Timestamp2FormatDateTime(time / 1000).c_str(), (unsigned int)time % 1000);
		return string(buf);
	}

	string CClock::RunInfo() const
	{
		unsigned long currentTime = MilliTimestamp();
		string head = "[" + m_tips + "]";
		string info = head + "Begin time : " + MilliTimestamp2DateTime(m_begin);
		info += "\n" + head + "End time : " + MilliTimestamp2DateTime(currentTime);
		info += "\n" + head + "Use times : " + I2S(currentTime - m_begin) + "(ms).";

		return info;
	}
};



