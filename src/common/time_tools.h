#ifndef CTM_COMMON_TIME_TOOLS_H__
#define CTM_COMMON_TIME_TOOLS_H__

#include <time.h>
#include <string>

namespace ctm
{
	inline unsigned long Time() { return time(NULL); }
	
	unsigned long long UTime();

	unsigned long Time2UTime();
	
	enum TimeFmt
	{
		TFMT_0, //yyyy-MM-dd HH:mm:ss
		TFMT_1, //yyyy/MM/dd HH:mm:ss
		TFMT_2  //yyyyMMddHHmmss
	};

	std::string DateTime(int fmt = TFMT_0);

};

#endif
