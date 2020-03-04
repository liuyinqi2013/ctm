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
		static const unsigned int LOG_FILE_MIN_SIZE = 2;   //文件最大尺寸单位MB
		static const unsigned int LOG_FILE_MAX_SIZE = 500; //文件最大尺寸单位MB

		enum LogLevel
		{
			LOG_NORMAL = 0,
			LOG_DEBUG,
			LOG_WARN,
			LOG_ERROR
		};

		CLog(const std::string& logName = "Run", const std::string& logPath = "./", unsigned int fileMaxSize = 50);
		~CLog();

		void SetLogName(const std::string& logName);
		void SetLogPath(const std::string& logPath);
		void SetFileMaxSize(unsigned int fileMaxSize);
		
		bool Write(enum LogLevel level, const char* format, ...);

	private:

		void InitFileName();

		void FormatPath(const std::string& path);

		std::string LinePrefix(enum LogLevel level);

		long long GetFileSize(const std::string& fileName);

		void SwitchNextFile();

	private:
		std::string m_logName;
		std::string m_logPath;
		unsigned int m_logFileMaxSize; //MB
		FILE* m_stream;
		int m_Index;
		std::string m_date;
		std::string m_fileName;
		CMutex m_mutexLock;
		unsigned long m_logFileSize;
	};

#define DEBUG_LOG(format,...) CLog::GetInstance()->Write(CLog::LOG_DEBUG, "[%s:%d]:"format, __FILE__, __LINE__, ##__VA_ARGS__)
#define ERROR_LOG(format,...) CLog::GetInstance()->Write(CLog::LOG_ERROR, "[%s:%d]:"format, __FILE__, __LINE__, ##__VA_ARGS__)

#define FUNC_BEG() DEBUG_LOG("Begin...")
#define FUNC_END() DEBUG_LOG("End...")
		
};

#endif
