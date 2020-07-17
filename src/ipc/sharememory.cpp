
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "ipc_common.h"
#include "common/macro.h"
#include "sharememory.h"

namespace ctm
{
	CShareMemory::CShareMemory(int key) :
		m_iKey(key),
		m_strName(""),
		m_shmId(-1),
		m_Size(0),
		m_shmHead(NULL)	
	{
	
	}
	
	CShareMemory::CShareMemory(const std::string& name) :
		m_iKey(-1),
		m_strName(name),
		m_shmId(-1),
		m_Size(0),
		m_shmHead(NULL)			
	{
	}
	
	CShareMemory::~CShareMemory()
	{
	}

	bool CShareMemory::Open(int size)
	{
		if (m_shmId != -1) return true;

		if (m_iKey == -1 && m_strName.size() > 0)
		{
			m_iKey = IpcKeyId(m_strName.c_str());
		}

		if (m_iKey == -1)
		{
			fprintf(stderr, "key is -1\n");
			return false;	
		}
		
		m_shmId = shmget(m_iKey, size, IPC_CREAT | IPC_EXCL | 0644);
		if (m_shmId == -1)
		{
			fprintf(stderr, "errcode = %d, errmsg = %s\n", errno, strerror(errno));
			if (errno != EEXIST) {
				return false;
			} 

			m_shmId = shmget(m_iKey, size, 0644);
			if (m_shmId == -1) {
				fprintf(stderr, "errcode = %d, errmsg = %s\n", errno, strerror(errno));
				return false;
			} 
		}

		m_shmHead = (char*)shmat(m_shmId, NULL, 0);
		if (m_shmHead == NULL)
		{
			fprintf(stderr, "errcode = %d, errmsg = %s\n", errno, strerror(errno));
			return false;
		}

		GetSize();

		return true;
	}

	bool CShareMemory::Destroy()
	{
		if (m_shmId == -1)
		{
			fprintf(stderr, "shm_id is -1");
			return false;
		}

		if (shmctl(m_shmId, IPC_RMID, NULL) == -1)
		{
			fprintf(stderr, "errcode = %d, errmsg = %s\n", errno, strerror(errno));
			return false;
		}

		m_shmId = -1;
		
		return true;
	}

	void CShareMemory::GetSize()
	{
	 	struct shmid_ds buf = {0};
		if (shmctl(m_shmId, IPC_STAT, &buf) == -1)
		{
			fprintf(stderr, "errcode = %d, errmsg = %s\n", errno, strerror(errno));
			return ;
		}
		m_Size = buf.shm_segsz;
	}

}

