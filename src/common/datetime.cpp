#include "datetime.h"

namespace ctm
{
	std::string CDateTime::Comm(CommFmt type)
	{
		struct tm* st = localtime(&m_timeT);
		char buf[64] = {0};
		switch (type)
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
	
	std::string CDateTime::ToString(const char* format)
	{
		struct tm* st = localtime(&m_timeT);
		char buf[64] = {0};
		strftime(buf, sizeof(buf), format, st);
		return std::string(buf);
	}

	
}
