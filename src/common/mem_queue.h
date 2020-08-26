#ifndef CTM_COMMON_MEM_QUEUE_H__
#define CTM_COMMON_MEM_QUEUE_H__


#include <stdint.h>

namespace ctm
{
    class CMemoryQueue
    {
    public:

        struct MemQueueHead
        {
            volatile uint32_t semId;
            volatile uint32_t semNum;
            volatile uint32_t rdIdx; 
            volatile uint32_t wrIdx;
            volatile uint32_t size;
            volatile uint32_t freeSize; 
        };

        CMemoryQueue(uint32_t semKey, uint32_t size);

        CMemoryQueue(void* addr, uint32_t size, int semId, int semNum, bool flush = false);

        ~CMemoryQueue();

        int Push(void* buf, uint32_t len);
        int Get(void* buf, uint32_t len);

        int Count();

        int Size();

    private:

        int Open(void* addr, uint32_t size, int semId, int semNum, bool flush = false);

        int Create(uint32_t semKey, uint32_t maxSize);

        int SemOp(int val, bool undo = true, uint32_t millTimeOut = 0);

    private:
        void* m_addr;
        MemQueueHead* m_head;
        void*    m_dataHead;
        bool     m_bCreate;
        uint32_t m_semKey;
    };
}

#endif