#include "time_tools.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#include <sys/time.h>
#endif

namespace ctm
{
	unsigned long long UTime()
	{
#ifndef WIN32
		struct timeval val = {0};
		gettimeofday(&val, NULL);
		return (val.tv_sec * 1000000 + val.tv_usec);
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

};



