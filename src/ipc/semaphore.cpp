#include "semaphore.h"
#include "common/macro.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>


namespace ctm
{
	union semun   
	{  
	    int val;  
	    struct semid_ds *buf;  
	    unsigned short  *array;  
	    struct seminfo  *__buf;   
	};
	
	CSemaphore::CSemaphore(int key) :
		m_iKey(key),
		m_strName(""),
		m_semHandle(-1),
		m_val(0)
	{
	}
	
	CSemaphore::CSemaphore(const std::string& name) :
		m_iKey(-1),
		m_strName(name),
		m_semHandle(-1),
		m_val(0)
	{
	}
	
	CSemaphore::~CSemaphore()
	{
	}
		
	bool CSemaphore::Open()
	{
		if (m_semHandle != -1) return true;

		if (m_iKey == -1 && m_strName.size() > 0)
		{
			m_iKey = KeyId(m_strName);
		}

		if (m_iKey == -1)
		{
			ERROR_LOG("key is -1");
			ERROR_LOG("errcode = %d, errmsg = %s", errno, strerror(errno));
			return false;	
		}

		m_semHandle = semget(m_iKey, 1, IPC_CREAT | 0644);
		if (m_semHandle == -1)
		{
			ERROR_LOG("errcode = %d, errmsg = %s", errno, strerror(errno));
			return false;
		}
	
		return true;
	}

	bool CSemaphore::SetVal(int val)
	{
		if (m_semHandle == -1)
		{
			ERROR_LOG("sem_id is -1");
			return false;
		}
				
		union semun un;
		un.val = val;
		if (semctl(m_semHandle, 0, SETVAL, un) == -1)
		{
			ERROR_LOG("errcode = %d, errmsg = %s", errno, strerror(errno));
			return false;
		}
		
		m_val = val;

		return true;
	}

	bool CSemaphore::Destroy()
	{
		if (m_semHandle == -1)
		{
			ERROR_LOG("sem_id is -1");
			return false;
		}

		if (semctl(m_semHandle, 0, IPC_RMID) == -1)
		{
			ERROR_LOG("errcode = %d, errmsg = %s", errno, strerror(errno));
			return false;
		}

		m_semHandle == -1;

		return true;
	}
		
	bool CSemaphore::P()
	{
		return PV(1);
	}
	
	bool CSemaphore::V()
	{
		return PV(-1);
	}

	bool CSemaphore::PV(int opt)
	{
		if (m_semHandle == -1)
		{
			ERROR_LOG("sem_id is -1");
			return false;
		}
		
		struct sembuf buf;  
    	buf.sem_num = 0;  
    	buf.sem_op = opt;  
    	buf.sem_flg = 0;  
  
    	if (semop(m_semHandle, &buf, 1) == -1)
    	{
			ERROR_LOG("errcode = %d, errmsg = %s", errno, strerror(errno));
			return false;    	
    	}
		
		return true;	
	}

	int CSemaphore::KeyId(const std::string& name)
	{
		struct stat buf = {0};
		std::string filePathName = "/tmp/" + name;
		if (stat(filePathName.c_str(), &buf) == -1)
		{
			FILE* fp = fopen(filePathName.c_str(), "wb");
			if (!fp)
			{
				ERROR_LOG("create file %s failed", filePathName.c_str());
				return -1;
			}
			fclose(fp);
		}
		
		return ftok(filePathName.c_str(), 0x666);
	}
}
