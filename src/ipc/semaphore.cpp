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
	CSemaphore::CSemaphore(int key, int semSize) :
		m_iKey(key),
		m_strName(""),
		m_semHandle(-1),
		m_semSize(semSize)
	{
	}
	
	CSemaphore::CSemaphore(const std::string& name, int semSize) :
		m_iKey(-1),
		m_strName(name),
		m_semHandle(-1),
		m_semSize(semSize)
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
			fprintf(stderr, "key is -1\n");
			return false;	
		}

		m_semHandle = semget(m_iKey, m_semSize, IPC_CREAT | 0644);
		if (m_semHandle == -1)
		{
			fprintf(stderr, "CSemaphore::Open errcode = %d, errmsg = %s\n", errno, strerror(errno));
			return false;
		}

		return true;
	}

	bool CSemaphore::SetVal(int val, int semNum)
	{
		if (m_semHandle == -1)
		{
			fprintf(stderr, "sem_id is -1");
			return false;
		}
				
		union semun un;
		un.val = val;
		if (semctl(m_semHandle, semNum, SETVAL, un) == -1)
		{
			fprintf(stderr, "errcode = %d, errmsg = %s\n", errno, strerror(errno));
			return false;
		}
		
		return true;
	}

	bool CSemaphore::Destroy()
	{
		if (m_semHandle == -1)
		{
			fprintf(stderr, "sem_id is -1");
			return false;
		}

		if (semctl(m_semHandle, 0, IPC_RMID) == -1)
		{
			fprintf(stderr, "errcode = %d, errmsg = %s\n", errno, strerror(errno));
			return false;
		}

		m_semHandle = -1;

		return true;
	}
		
	bool CSemaphore::Post(int semNum)
	{
		return SemOpt(1, semNum);
	}
	
	bool CSemaphore::Wait(int semNum)
	{
		return SemOpt(-1, semNum);
	}

	bool CSemaphore::SemOpt(int opt, int semNum)
	{
		if (m_semHandle == -1)
		{
			fprintf(stderr, "sem_id is -1\n");
			return false;
		}
		
		struct sembuf buf;  
    	buf.sem_num = semNum;  
    	buf.sem_op  = opt;  
    	buf.sem_flg = SEM_UNDO;  
    	if (semop(m_semHandle, &buf, 1) == -1)
    	{
			fprintf(stderr, "errcode = %d, errmsg = %s\n", errno, strerror(errno));
			return false;    	
    	}
		
		return true;	
	}
}
