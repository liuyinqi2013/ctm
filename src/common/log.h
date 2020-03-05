#ifndef CTM_COMMON_LOG_H__
#define CTM_COMMON_LOG_H__
#include <stdio.h>
#include <string>
#include "thread/mutex.h"
#include "singleton.h"


// ����Ĭ��ֵ
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
		CLog�๹�ɺ���
		logName����־�ļ����ƣ�Ĭ��Ϊ��׼���
		logPath����־�ļ����·��
		fileMaxSize����־�ļ���С����λMB
		level����־�ļ��ȼ���С��level�ȼ�����־������¼
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

		// �ݹ鴴���ļ���
		static void MakePath(const std::string& path);

	private:

		void Init();

		void AssignPath(const std::string& path);
		void AssignMaxSize(unsigned int fileMaxSize);

		// ǰ׺��ʽ[datatime][pid][level]
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

#define DEBUG_LOG(format,...) CLog::GetInstance()->Write(CLog::LOG_DEBUG, "[%s:%d]:" format, __FILE__, __LINE__, ##__VA_ARGS__)
#define ERROR_LOG(format,...) CLog::GetInstance()->Write(CLog::LOG_ERROR, "[%s:%d]:" format, __FILE__, __LINE__, ##__VA_ARGS__)
#define WARN_LOG(format,...) CLog::GetInstance()->Write(CLog::LOG_WARN, "[%s:%d]:" format, __FILE__, __LINE__, ##__VA_ARGS__)
#define INFO_LOG(format,...) CLog::GetInstance()->Write(CLog::LOG_INFO, "[%s:%d]:" format, __FILE__, __LINE__, ##__VA_ARGS__)

#define FUNC_BEG() DEBUG_LOG("Begin...")
#define FUNC_END() DEBUG_LOG("End...")

};

#endif
