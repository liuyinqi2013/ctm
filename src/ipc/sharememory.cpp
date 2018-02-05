#include "sharememory.h"
#include "common/macro.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

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

	bool CShareMemory::Create(int size)
	{
		if (m_shmId != -1) return true;

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

		m_shmId = shmget(m_iKey, size, 0644);
		if (m_shmId != -1)
		{
			if (!Destroy())
			{
				ERROR_LOG("errcode = %d, errmsg = %s", errno, strerror(errno));
				return false;
			}
		}

		m_shmId = shmget(m_iKey, size,  IPC_CREAT | IPC_EXCL| 0644);
		if (m_shmId == -1)
		{
			ERROR_LOG("errcode = %d, errmsg = %s", errno, strerror(errno));
			return false;			
		}

		m_shmHead = (char*)shmat(m_shmId, NULL, 0);
		if (m_shmHead == NULL)
		{
			ERROR_LOG("errcode = %d, errmsg = %s", errno, strerror(errno));
			return false;
		}		

		GetSize();
		
		return true;
	}

	bool CShareMemory::Open(int size, bool creat)
	{
		if (m_shmId != -1) return true;

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

		int mode = 0644;

		if (creat) mode |= IPC_CREAT;
		
		m_shmId = shmget(m_iKey, size, mode);
		if (m_shmId == -1)
		{
			ERROR_LOG("errcode = %d, errmsg = %s", errno, strerror(errno));
			return false;
		}

		m_shmHead = (char*)shmat(m_shmId, NULL, 0);
		if (m_shmHead == NULL)
		{
			ERROR_LOG("errcode = %d, errmsg = %s", errno, strerror(errno));
			return false;
		}		

		GetSize();
		
		return true;
	}

	bool CShareMemory::Destroy()
	{
		if (m_shmId == -1)
		{
			ERROR_LOG("shm_id is -1");
			return false;
		}

		if (shmctl(m_shmId, IPC_RMID, NULL) == -1)
		{
			ERROR_LOG("errcode = %d, errmsg = %s", errno, strerror(errno));
			return false;
		}

		m_shmId = -1;
		
		return true;
	}

	int CShareMemory::KeyId(const std::string& name)
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

	void CShareMemory::GetSize()
	{
	 	struct shmid_ds buf = {0};
		if (shmctl(m_shmId, IPC_STAT, &buf) == -1)
		{
			ERROR_LOG("errcode = %d, errmsg = %s", errno, strerror(errno));
			return ;
		}

		m_Size = buf.shm_segsz;
	}

}

