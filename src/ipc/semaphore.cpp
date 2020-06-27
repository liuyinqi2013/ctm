#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "semaphore.h"
#include "ipc_common.h"
#include "common/macro.h"

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
			m_iKey = IpcKeyId(m_strName.c_str());
		}

		if (m_iKey == -1)
		{
			ERROR_LOG("key is -1");
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

		m_semHandle = -1;

		return true;
	}
		
	bool CSemaphore::Post()
	{
		return SemOpt(1);
	}
	
	bool CSemaphore::Wait()
	{
		return SemOpt(-1);
	}

	bool CSemaphore::SemOpt(int opt)
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
}
