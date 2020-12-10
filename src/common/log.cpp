#include "log.h"
#include "com.h"
#include "time_tools.h"
#include "lock.h"
#include "platform.h"

#include <sys/stat.h>
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>

#define MB (1024 * 1024)
#define GB (1024 * 1024 * 1024)
#ifdef _MSC_VER
	#define MKDIR "mkdir "
#else
	#define MKDIR "mkdir -p "
#endif

namespace ctm
{
	CLog* gErrorLog = CLog::GetInstance();
	CLog* gDebugLog = CLog::GetInstance();
	CLog* gWarnLog  = CLog::GetInstance();

	void CLog::MakePath(const std::string& path)
	{
		std::string cmd(MKDIR + std::string("\"") + path + std::string("\""));
		assert(system(cmd.c_str()) == 0);
	}

	CLog::CLog(const std::string& logName, 
		const std::string& logPath, 
		unsigned int fileMaxSize,
		enum LogLevel level) :
		m_logName(logName),
		m_logPath(logPath),
		m_logFileMaxSize(fileMaxSize),
		m_stream(NULL),
		m_Index(0),
		m_date(""),
		m_logFileSize(0),
		m_logLevel(level)
	{
		AssignPath(logPath);
		AssignMaxSize(fileMaxSize);
		Init();
	}

	CLog::~CLog()
	{
		CloseFile();
	}

	void CLog::SetLogName(const std::string& logName)
	{
		m_logName = logName;
		Init();
	}

	void CLog::SetLogPath(const std::string& logPath)
	{
		AssignPath(logPath);
		Init();
	}

	void CLog::SetFileMaxSize(unsigned int fileMaxSize)
	{
		AssignMaxSize(fileMaxSize);
		Init();
	}

	void CLog::AssignMaxSize(unsigned int fileMaxSize)
	{
		if (m_logFileMaxSize < LOG_FILE_MIN_SIZE)
		{
			m_logFileMaxSize = LOG_FILE_MIN_SIZE;
		}
		else if (m_logFileMaxSize > LOG_FILE_MAX_SIZE)
		{
			m_logFileMaxSize = LOG_FILE_MAX_SIZE;
		}
		else
		{
			m_logFileMaxSize = fileMaxSize;
		}
	}

	void CLog::SetLogLevel(enum LogLevel level)
	{
		m_logLevel = level;
	}

	void CLog::Init()
	{
		if (m_logName != STDOUT_LOG_FILE) 
		{
			m_Index = 0;
			m_date = Date(TDATE_FMT_2);
			m_fileName = m_logPath + m_logName + "_" + m_date + ".log";
			m_logFileSize = GetFileSize(m_fileName);

			while (m_logFileSize >= m_logFileMaxSize * MB)
			{
				ToNextFile();
			}
		}
	}

	long long CLog::GetFileSize(const std::string& fileName)
	{
		struct stat st;
		return (stat(m_fileName.c_str(), &st) == 0) ? st.st_size : 0;
	}

	void CLog::ToNextFile()
	{
		m_fileName = m_logPath + m_logName + "_" + m_date + "_" + I2S(++m_Index) + ".log";
		m_logFileSize = GetFileSize(m_fileName);
	}

	std::string CLog::MilliSecondStr()
	{
		char buf[4] = { 0 };
		snprintf(buf, sizeof(buf), "%03d", MilliSeconds());
		return std::string(buf);
	}

	void CLog::OpenFile()
	{
		CloseFile();
		if (m_logName != STDOUT_LOG_FILE)
		{ 
			m_stream = fopen(m_fileName.c_str(), "a+");
			assert(m_stream != NULL);
		}
		else 
		{
			m_stream = stdout;
		}
	}

	void CLog::CloseFile()
	{
		if (m_stream && m_stream != stdout)
		{
			fclose(m_stream);
			m_stream = NULL;
		}
	}

	void CLog::AssignPath(const std::string& path)
	{
		m_logPath = path;
		if (path.size() == 0)
		{
			m_logPath += "./";
		}
		else if (*path.rbegin() != '/')
		{
			m_logPath += "/";
		}

		MakePath(m_logPath);
	}

	std::string CLog::LinePrefix(enum LogLevel level)
	{
		std::string prefix = "[" + DateTime() + "." + MilliSecondStr() +"][" + I2S(GetPid()) + "][";
		switch (level)
		{
		case LOG_DEBUG:
			prefix += "DEBUG";
			break;
		case LOG_WARN:
			prefix += "WARN";
			break;
		case LOG_ERROR:
			prefix += "ERROR";
			break;
		case LOG_INFO:
			prefix += "INFO";
			break;
		default:
			prefix += "UNKNOWN";
		}
		prefix += "]";
		return prefix;
	}

	bool CLog::Write(enum LogLevel level, const char* format, ...)
	{
		if (m_logLevel > level || level == LOG_NOLOG)
		{
			return true;
		}

		CLockOwner owner(m_mutexLock);

		if (m_logName != STDOUT_LOG_FILE)
		{
			// 检查文件切换
			if (m_date != Date(TDATE_FMT_2))
			{
				Init();
			}
			else if (m_logFileSize >= m_logFileMaxSize * MB)
			{
				ToNextFile();
			}
		}

		if (m_stream == NULL)
		{
			OpenFile();
		}
		std::string strFmt = LinePrefix(level) + format + std::string("\n");
		va_list args;
		va_start(args, format);
		m_logFileSize += vfprintf(m_stream, strFmt.c_str(), args);
		va_end(args);
		
		fflush(m_stream);

		return true;
	}
};
