#include "time_tools.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#include <sys/time.h>
#endif

namespace ctm
{
	unsigned long long Usec()
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
			sprintf(buf, "%4d-%02d-%02d %02d:%02d:%02d",  
				st->tm_year + 1900,
				st->tm_mon + 1,
				st->tm_mday,
				st->tm_hour,
				st->tm_min,
				st->tm_sec);
			break;
		case TFMT_1:
			sprintf(buf, "%4d/%02d/%02d %02d:%02d:%02d",  
				st->tm_year + 1900,
				st->tm_mon + 1,
				st->tm_mday,
				st->tm_hour,
				st->tm_min,
				st->tm_sec);
			break;
		case TFMT_2:
			sprintf(buf, "%4d%02d%02d%02d%02d%02d",  
				st->tm_year + 1900,
				st->tm_mon + 1,
				st->tm_mday,
				st->tm_hour,
				st->tm_min,
				st->tm_sec);
			break;
		default:			
			sprintf(buf, "%4d-%02d-%02d %02d:%02d:%02d",  
				st->tm_year + 1900,
				st->tm_mon + 1,
				st->tm_mday,
				st->tm_hour,
				st->tm_min,
				st->tm_sec);			
			break;
		}

		return std::string(buf);
	}

};



