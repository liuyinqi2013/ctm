#ifndef CTM_COMMON_LOG_H__
#define CTM_COMMON_LOG_H__
#include <stdio.h>
#include <string>

#include "thread/mutex.h"
#include "singleton.h"

// 定义默认值
#define LOG_FILE_MIN_SIZE (2)
#define LOG_FILE_MAX_SIZE (500)
#define LOG_FILE_DEFAULT_SIZE (50) 
#define STDOUT_LOG_FILE "STDOUT"
#define LOG_FILE_DEFAULT_NAME STDOUT_LOG_FILE
#define LOG_FILE_DEFAULT_PATH "./"

namespace ctm
{
	class CLog : public CSingleton<CLog>
	{
	public:
		enum LogLevel
		{
			LOG_DEBUG = 0,
			LOG_INFO,
			LOG_WARN,
			LOG_ERROR,
			LOG_NOLOG,
		};

		/*
		CLog类构成函数
		logName：日志文件名称，默认为标准输出
		logPath：日志文件存放路径
		fileMaxSize：日志文件大小，单位MB
		level：日志文件等级，小于level等级的日志将不记录
		*/
		CLog(const std::string& logName = LOG_FILE_DEFAULT_NAME,
			const std::string& logPath = LOG_FILE_DEFAULT_PATH,
			unsigned int fileMaxSize = LOG_FILE_DEFAULT_SIZE,
			enum LogLevel level = LOG_DEBUG);

		~CLog();

		void SetLogName(const std::string& logName);
		void SetLogPath(const std::string& logPath);
		void SetFileMaxSize(unsigned int fileMaxSize);
		void SetLogLevel(enum LogLevel level);

		bool Write(enum LogLevel level, const char* format, ...);

		// 递归创建文件夹
		static void MakePath(const std::string& path);

	private:

		void Init();

		void AssignPath(const std::string& path);
		void AssignMaxSize(unsigned int fileMaxSize);

		// 前缀格式[datatime][pid][level]
		std::string LogDateFmt();
		std::string LinePrefix(enum LogLevel level);
		long long GetFileSize(const std::string& fileName);
		void ToNextFile();
		std::string MilliSecondStr();

		void OpenFile();
		void CloseFile();

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
		int m_logLevel;
	};
};

extern ctm::CLog* gErrorLog;
extern ctm::CLog* gDebugLog;
extern ctm::CLog* gWarnLog;

#define CTM_DEBUG_LOG(log, format, ...) if (log) (log)->Write(ctm::CLog::LOG_DEBUG, "[%s:%d]:" format, __FILE__, __LINE__, ##__VA_ARGS__)
#define CTM_ERROR_LOG(log, format, ...) if (log) (log)->Write(ctm::CLog::LOG_ERROR, "[%s:%d]:" format, __FILE__, __LINE__, ##__VA_ARGS__)
#define CTM_WARN_LOG(log, format, ...)  if (log) (log)->Write(ctm::CLog::LOG_WARN,  "[%s:%d]:" format, __FILE__, __LINE__, ##__VA_ARGS__)
#define CTM_INFO_LOG(log, format, ...)  if (log) (log)->Write(ctm::CLog::LOG_INFO,  "[%s:%d]:" format, __FILE__, __LINE__, ##__VA_ARGS__)

#define CTM_FUNC_BEG(log) CTM_DEBUG_LOG(log, "Begin...")
#define CTM_FUNC_END(log) CTM_DEBUG_LOG(log, "End...")

#define DEBUG(format, ...) CTM_DEBUG_LOG(gDebugLog, format, ##__VA_ARGS__)
#define ERROR(format, ...) CTM_ERROR_LOG(gErrorLog, format, ##__VA_ARGS__)
#define WARN(format, ...)  CTM_WARN_LOG(gWarnLog, format, ##__VA_ARGS__)
#define INFO(format, ...)  CTM_INFO_LOG(gWarnLog, format, ##__VA_ARGS__)

#define FUNC_BEG() DEBUG("...BEG...")
#define FUNC_END() DEBUG("...END...")

#endif
