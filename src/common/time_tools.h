#ifndef CTM_COMMON_TIME_TOOLS_H__
#define CTM_COMMON_TIME_TOOLS_H__

#include <time.h>
#include <string>

namespace ctm
{
	inline unsigned long Sec() { return time(NULL); }
	
	unsigned long long Usec();

	unsigned long Sec2Usec();
	
	enum TimeFmt
	{
		TFMT_0, //yyyy-MM-dd HH:mm:ss
		TFMT_1, //yyyy/MM/dd HH:mm:ss
		TFMT_2  //yyyyMMddHHmmss
	};

	std::string DateTime(int fmt = TFMT_0);

};

#endif
