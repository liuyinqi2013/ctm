#include "time_tools.h"
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
	unsigned long MilliTime()
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

	std::string DateTime(int fmt)
	{
		time_t t;
		time(&t);
		struct tm* st = localtime(&t);
		char buf[64] = {0};
		switch (fmt)
		{
		case TFMT_0:
			strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", st);
			break;
		case TFMT_1:
			strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M:%S", st);
			break;
		case TFMT_2:
			strftime(buf, sizeof(buf), "%Y%m%d%H%M%S", st);
			break;
		default:
			strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", st);
			break;
		}

		return std::string(buf);
	}

	std::string Date(int fmt)
	{
		time_t t;
		time(&t);
		struct tm* st = localtime(&t);
		char buf[64] = {0};
		switch (fmt)
		{
		case TFMT_0:
			strftime(buf, sizeof(buf), "%Y-%m-%d", st);
			break;
		case TFMT_1:
			strftime(buf, sizeof(buf), "%Y/%m/%d", st);
			break;
		case TFMT_2:
			strftime(buf, sizeof(buf), "%Y%m%d", st);
			break;
		default:
			strftime(buf, sizeof(buf), "%Y-%m-%d", st);
			break;
		}

		return std::string(buf);
	}

};



