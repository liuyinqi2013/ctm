 #include "log.h"
#include "com.h"
#include "time_tools.h"
#include "lock.h"
#include "platform.h"



#include <sys/stat.h>
#include <stdarg.h>


namespace ctm
{
	CLog::CLog(const std::string& logName, const std::string& logPath, unsigned int fileMaxSize, bool onlyBack) :
		m_logName(logName),
		m_logPath(logPath),
		m_logFileMaxSize(fileMaxSize),
		m_stream(NULL),
		m_Index(0),
		m_date(Date(TFMT_2)),
		m_onlyBack(onlyBack)
	{
		if (m_logFileMaxSize == 0)
		{
			m_logFileMaxSize = 1;
		}
		
		if (m_logPath.size() == 0 || *m_logPath.rbegin() != '/')
		{
			m_logPath += "/";
		}

		m_fileNamePrefix = m_logPath + m_logName + "_" + m_date;
		
		InitFileName();
	}
	
	CLog::~CLog()
	{
	}

	void CLog::SetLogName(const std::string& logName)
	{
		CLockOwner owner(m_mutexLock);
		m_logName = logName;
		
		m_fileNamePrefix = m_logPath + m_logName + "_" + m_date;
		InitFileName();
	}
	
	void CLog::SetLogPath(const std::string& logPath)
	{
		CLockOwner owner(m_mutexLock);

		m_logPath = logPath;
		if (m_logPath.size() == 0 || *m_logPath.rbegin() != '/')
		{
			m_logPath += "/";
		}

		m_fileNamePrefix = m_logPath + m_logName + "_" + m_date;
		InitFileName();
		
	}
	
	void CLog::SetFileMaxSize(unsigned int fileMaxSize)
	{
		CLockOwner owner(m_mutexLock);
		m_logFileMaxSize = fileMaxSize;
	}
	
	void CLog::SetOnlyBack(bool onlyBack)
	{
		CLockOwner owner(m_mutexLock);
		m_onlyBack = onlyBack;
	}

	void CLog::InitFileName()
	{
		m_Index = 0;
		struct stat st;
		m_fileName = m_fileNamePrefix + ".log";
		while(stat(m_fileName.c_str(), &st) == 0 && st.st_size > 1024 * 1024 * m_logFileMaxSize)
		{
			m_fileName = m_fileNamePrefix + "_" + I2S(++m_Index) + ".log";
		}
	}
	
	bool CLog::Write(enum LogLevel level, const char* format, ...)	
	{
		if (LOG_NORMAL == level)
		{
			return true;
		}

		CLockOwner owner(m_mutexLock);
		
		std::string currDate = Date(TFMT_2);
		if (m_date != currDate)
		{
			m_Index = 0;
			m_date = currDate;
			m_fileNamePrefix = m_logPath + m_logName + "_" + m_date;
			m_fileName = m_fileNamePrefix + ".log";
		}

		struct stat st;
		if (stat(m_fileName.c_str(), &st) == 0 && st.st_size > 1024 * 1024 * m_logFileMaxSize)
		{
			m_fileName = m_fileNamePrefix + "_" + I2S(++m_Index) + ".log";
		}

		m_stream = fopen(m_fileName.c_str(), "a+");
		if (NULL == m_stream)
		{
			return false;
		}

		std::string strFmt = "[" + DateTime() + "][" + I2S(GetPid()) + "][";
		switch(level)
		{
		case LOG_DEBUG:
			strFmt += "debug";
			break;
		case LOG_WARN:
			strFmt += "warn";
			break;
		case LOG_ERROR:
			strFmt += "error";
			break;
		default:
			strFmt += "debug";
		}
		strFmt += "]";
		strFmt += format;
		strFmt += "\n";

		va_list args;
  		va_start(args, format);
		
		if(!m_onlyBack)
		{
			va_list args1;
			va_copy(args1, args);
			vfprintf(stdout, strFmt.c_str(), args1);
			va_end(args1);
		}
		
  		vfprintf(m_stream, strFmt.c_str(), args);
		fclose(m_stream);

  		va_end(args);
		
		return true;
	}
};
