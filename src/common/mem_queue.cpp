
#include "mem_queue.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "ipc/semaphore.h"


#define DATA_GAP 32

namespace ctm
{
    CMemoryQueue::CMemoryQueue(uint32_t semKey, uint32_t size):
        m_addr(NULL),
        m_head(NULL),
        m_dataHead(NULL),
        m_bCreate(false)
    {
        Create(semKey, size);
    }


    CMemoryQueue::CMemoryQueue(void* addr, uint32_t size, int semId, int semNum, bool flush):
        m_addr(addr),
        m_head(NULL),
        m_dataHead(NULL),
        m_bCreate(false)
    {
        Open(addr, size, semId, semNum, flush);
    }

    CMemoryQueue::~CMemoryQueue()
    {
        if (m_bCreate)
        {
            if (m_head->semId)
            {
                semctl(m_head->semId, 0, IPC_RMID);
            }

            if (m_addr) free(m_addr);
        }
    }

    int CMemoryQueue::Push(void* buf, uint32_t len)
    {
        SemOp(-1);
        if ((int)m_head->freeSize <= 0 || len == 0)
        {
            SemOp(1);
            return 0;
        }

        uint32_t n = len > m_head->freeSize ? m_head->freeSize : len;

        if (m_head->wrIdx < m_head->rdIdx)
        {
            memmove((char*)m_dataHead + m_head->wrIdx, buf, n);
        }
        else 
        {
            uint32_t m = m_head->size - m_head->wrIdx;
            if (m >= n)
            {
                memmove((char*)m_dataHead + m_head->wrIdx, buf, n);
            }
            else
            {
                memmove((char*)m_dataHead + m_head->wrIdx, buf, m);
                memmove((char*)m_dataHead, (char*)buf + m, n - m);
            }
        }

        m_head->wrIdx = (m_head->wrIdx + n) % m_head->size;
        m_head->freeSize -= n;

        SemOp(1);

        return n;
    }

    int CMemoryQueue::Get(void* buf, uint32_t len)
    {
        SemOp(-1);
        if (m_head->freeSize == m_head->size || len == 0)
        {
            SemOp(1);
            return 0;
        }

        uint32_t l = m_head->size -  m_head->freeSize;

        uint32_t n = len > l ? l : len;

        if (m_head->rdIdx < m_head->wrIdx)
        {
            memmove(buf, (char*)m_dataHead + m_head->rdIdx, n);
        }
        else 
        {
            uint32_t m = m_head->size - m_head->rdIdx;
            if (m >= n)
            {
                memmove(buf, (char*)m_dataHead + m_head->rdIdx, n);
            }
            else
            {
                memmove(buf, (char*)m_dataHead + m_head->rdIdx, m);
                memmove((char*)buf + m, (char*)m_dataHead, n - m);
            }
        }

        m_head->rdIdx = (m_head->rdIdx + n) % m_head->size;
        
        m_head->freeSize += n;
        SemOp(1);

        return n;
    }

    int CMemoryQueue::Count()
    {
        /*
        int cnt = 0;

        SemOp(-1);
        cnt = m_head->size - m_head->freeSize;
        SemOp(1);
        */

        return m_head->size - m_head->freeSize;;
    }

    int CMemoryQueue::Size()
    {
        return m_head->size;
    }

    int CMemoryQueue::Create(uint32_t semKey, uint32_t size)
    {
        m_bCreate = true;

        m_addr = malloc(size + DATA_GAP);

        if (m_addr == NULL)
        {
            return -1;
        }

        int m_semId = semget(semKey, 1, IPC_CREAT | 0644);
		if (m_semId == -1)
		{
			fprintf(stderr, "errcode = %d, errmsg = %s\n", errno, strerror(errno));
			return -1;
		}

        int m_semNum = 0;

        union semun un;
		un.val = 1;
		if (semctl(m_semId, m_semNum, SETVAL, un) == -1)
		{
			fprintf(stderr, "errcode = %d, errmsg = %s\n", errno, strerror(errno));
			return -1;
		}
         
        m_head = (MemQueueHead*)m_addr;
        m_head->semId  = m_semId;
        m_head->semNum = m_semNum;
        m_head->rdIdx  = 0;
        m_head->wrIdx  = 0;
        m_head->size   = size - sizeof(MemQueueHead);
        m_head->freeSize = m_head->size;

        m_dataHead = (char*)m_addr + sizeof(MemQueueHead);

        return 0;
    }

    int CMemoryQueue::Open(void* addr, uint32_t size, int semId, int semNum, bool flush)
    {
        if (addr == NULL || size < sizeof(MemQueueHead))
        {
            return -1;
        }

        m_head = (MemQueueHead*)addr;
        m_dataHead = (char*)addr + sizeof(MemQueueHead);

        if (flush || 
            m_head->size == 0 || 
            (int)m_head->freeSize < 0 || 
            (int)m_head->freeSize > size - sizeof(MemQueueHead))
        {
            m_head->semId  = semId;
            m_head->semNum = semNum;
            m_head->rdIdx  = 0;
            m_head->wrIdx  = 0;
            m_head->size   = size - sizeof(MemQueueHead);
            m_head->freeSize = m_head->size;
        }

        return 0;
    }

    int CMemoryQueue::SemOp(int val, bool undo, uint32_t millTimeOut)
    {
        struct sembuf buf;  
    	buf.sem_num = m_head->semNum;  
    	buf.sem_op  = val;  
    	buf.sem_flg = 0; 
        if (undo) buf.sem_flg |= SEM_UNDO;

        int err = 0;

        while(1)
        {
            if (semop(m_head->semId, &buf, 1) == -1)
            {
                err = errno;

                if (err == EINTR) continue;
                else if (err == EAGAIN) return 1;

                fprintf(stderr, "errcode = %d, errmsg = %s\n", err, strerror(err));

                return -1;    	
            }

            break;
        }

        return 0;
    }
}