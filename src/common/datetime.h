#ifndef CTM_COMMON_DATE_TIME_H__
#define CTM_COMMON_DATE_TIME_H__

#include <time.h>
#include <string>

namespace ctm
{
	class CDateTime
	{
	public:
		enum CommFmt
		{
			TFMT_0, //yyyy-MM-dd HH:mm:ss
			TFMT_1, //yyyy/MM/dd HH:mm:ss
			TFMT_2	//yyyyMMddHHmmss
		};
		
		CDateTime(time_t t = time(NULL)) : m_timeT(t) {}
		CDateTime(const CDateTime& other) : m_timeT(other.m_timeT) {}
		~CDateTime(){}

		time_t UnixTime() { return m_timeT; }
		std::string Comm(CommFmt type = TFMT_0);
		std::string ToString(const char* format);
		bool FromString(const char* format, const char* strTime);
	private:
		time_t m_timeT;
	};
}

#endif

