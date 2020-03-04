#include "log.h"
#include "com.h"
#include "time_tools.h"
#include "lock.h"
#include "platform.h"

#include <sys/stat.h>
#include <stdarg.h>

#define MB (1024 * 1024)
#define GB (1024 * 1024 * 1024)

namespace ctm
{
	CLog::CLog(const std::string& logName, const std::string& logPath, unsigned int fileMaxSize) :
		m_logName(logName),
		m_logPath(logPath),
		m_logFileMaxSize(fileMaxSize),
		m_stream(NULL),
		m_Index(0),
		m_date(Date(TFMT_2)),
		m_logFileSize(0)
	{
		FormatPath(m_logPath);
		SetFileMaxSize(fileMaxSize);
		InitFileName();	}

	CLog::~CLog()
	{
	}

	void CLog::SetLogName(const std::string& logName)
	{
		m_logName = logName;
		InitFileName();
	}

	void CLog::SetLogPath(const std::string& logPath)
	{
		m_logPath = logPath;
		FormatPath(m_logPath);
		InitFileName();
	}

	void CLog::SetFileMaxSize(unsigned int fileMaxSize)
	{
		if (m_logFileMaxSize < LOG_FILE_MIN_SIZE) 
		{
			m_logFileMaxSize = LOG_FILE_MIN_SIZE;
		}	
		else if (m_logFileMaxSize > LOG_FILE_MAX_SIZE) 
		{
			m_logFileMaxSize = LOG_FILE_MAX_SIZE;
		}

		m_logFileMaxSize = fileMaxSize;
	}

	void CLog::InitFileName()
	{
		m_Index = 0;
		m_fileName = m_logPath + m_logName + "_" + m_date + ".log";
		m_logFileSize = GetFileSize(m_fileName);
		while (m_logFileSize > m_logFileMaxSize * MB)
		{
			SwitchNextFile();
		}
	}

	long long CLog::GetFileSize(const std::string& fileName)
	{
		struct stat st;
		return (stat(m_fileName.c_str(), &st) == 0) ? st.st_size : 0;
	}

	void CLog::SwitchNextFile()
	{
		m_fileName = m_logPath + m_logName + "_" + m_date + "_" + I2S(++m_Index) + ".log";
		m_logFileSize = GetFileSize(m_fileName);
		if (m_stream)
		{
			fclose(m_stream);
			m_stream = NULL;
		}
	}

	void CLog::FormatPath(const std::string& path)
	{
		if (path.size() == 0)
		{
			m_logPath += "./";
		}
		else if (*path.rbegin() != '/')
		{
			m_logPath += "/";
		}
	}

	std::string CLog::LinePrefix(enum LogLevel level)
	{
		std::string prefix = "[" + DateTime() + "][" + I2S(GetPid()) + "][";
		switch (level)
		{
		case LOG_DEBUG:
			prefix += "debug";
			break;
		case LOG_WARN:
			prefix += "warn";
			break;
		case LOG_ERROR:
			prefix += "error";
			break;
		default:
			prefix += "debug";
		}
		prefix += "]";
		return prefix;
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
			m_fileName = m_logPath + m_logName + "_" + m_date + ".log";
			m_logFileSize = GetFileSize(m_fileName);
			if (m_stream) 
			{
				fclose(m_stream);
				m_stream = NULL;
			}
		}

		if (m_logFileSize > m_logFileMaxSize * MB)
		{
			SwitchNextFile();
		}

		if (NULL == m_stream)
		{
			m_stream = fopen(m_fileName.c_str(), "a+");
			if (NULL == m_stream){
				return false;
			}
		}

		std::string strFmt = LinePrefix(level) + format + std::string("\n");
		va_list args;
		va_start(args, format);
		int len = vfprintf(m_stream, strFmt.c_str(), args);
		m_logFileSize += len;
		fflush(m_stream);
		va_end(args);

		return true;
	}
};
