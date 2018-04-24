#ifndef CTM_COMMON_LOG_H__
#define CTM_COMMON_LOG_H__
#include <stdio.h>
#include <string>
#include "thread/mutex.h"
#include "singleton.h"



namespace ctm
{
	class CLog : public CSingleton<CLog>
	{
	public:
		enum LogLevel
		{
			LOG_NORMAL = 0,
			LOG_DEBUG,
			LOG_WARN,
			LOG_ERROR
		};

		CLog(const std::string& logName = "Run", const std::string& logPath = "./", unsigned int fileMaxSize = 50, bool onlyBack = false);
		~CLog();

		void SetLogName(const std::string& logName);
		void SetLogPath(const std::string& logPath);
		void SetFileMaxSize(unsigned int fileMaxSize);
		void SetOnlyBack(bool onlyBack);
		
		bool Write(enum LogLevel level, const char* format, ...);

	private:

		void InitFileName();

	private:
		std::string m_logName;
		std::string m_logPath;
		unsigned int m_logFileMaxSize; //MB
		FILE* m_stream;
		int m_Index;
		std::string m_date;
		std::string m_fileName;
		std::string m_fileNamePrefix;
		CMutex m_mutexLock;
		bool m_onlyBack;
		bool m_bFileChange;
	};
		
};

#endif
