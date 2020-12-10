#include "memconn.h"
#include "common/mem_queue.h"
#include "common/macro.h"
#include "ipc/semaphore.h"
#include "ipc/sharememory.h"

namespace ctm
{
    CShardMemConn::CShardMemConn() :
    m_bServer(false),
    m_sendQueue(NULL),
    m_recvQueue(NULL)
    {

    }
        
    CShardMemConn::~CShardMemConn()
    {
        DELETE(m_sendQueue);
        DELETE(m_recvQueue);
        DELETE(m_semaphore);
        DELETE(m_shareMem);
    }

     int CShardMemConn::Open(unsigned int key, unsigned int size, bool bServer)
     {
        unsigned int memSize = 2 * (size + 32);

        m_shareMem = new CShareMemory(key);
        if (m_shareMem == NULL)
        {
            ERROR("Create share memory failed!");
            return -1;
        }

        if (!m_shareMem->Open(memSize))
        {
            ERROR("Open share memory failed!");
            return -1;
        }

        m_semaphore = new CSemaphore(key, 2);
        if (m_semaphore == NULL)
        {
            ERROR("Create semaphore failed!");
            return -1;
        }

        if (!m_semaphore->Open())
        {
            ERROR("Open semaphore failed!");
            return -1;
        }

        if (bServer)
        {
            m_semaphore->SetVal(1, 0);
            m_semaphore->SetVal(1, 1);
        }

        int semId = m_semaphore->GetSemId();
        void* haed = m_shareMem->Head();


        CMemoryQueue* sendQueue = NULL;
        CMemoryQueue* recvQueue = NULL;
        
        unsigned int nsize = 32 + size;
        
        if (bServer)
        {
            sendQueue = new CMemoryQueue((char*)haed, size, semId, 0, true);
            recvQueue = new CMemoryQueue((char*)haed + nsize, size, semId, 1, true);
        }
        else
        {
            recvQueue = new CMemoryQueue((char*)haed, size, semId, 0);
            sendQueue = new CMemoryQueue((char*)haed + nsize, size, semId, 1);
        }
        
        this->m_bServer = bServer;
        this->m_sendQueue = sendQueue;
        this->m_recvQueue = recvQueue;

        return 0;
     }

    void CShardMemConn::Reset()
    {
        m_bServer = false;
        m_sendQueue = NULL;
        m_recvQueue = NULL;

        CConn::Reset();
    }

    void CShardMemConn::CloseRead()
    {
    }

    void CShardMemConn::CloseWrite()
    {
    }

    void CShardMemConn::Close()
    {
        m_shareMem->Dettch();
        ChangeStatus(CLOSED);
    }

    int CShardMemConn::Send(Buffer* buf)
    {
        writable = false;
        time_t now = time(NULL);

        if (status == WRCLOSED || status >= HANGUP)
        {
            DEBUG("Conn can not write status : %d", status);

            return IO_NO_WRITE;
        }

        int ret = m_sendQueue->Push(buf->data + buf->offset, buf->len - buf->offset);

        buf->offset += ret;

        if ((unsigned int)ret == buf->len - buf->offset)
        {
            writable = true;
            lastWrite = now;
            lastActive = now;

            return IO_WR_OK;
        }
        else
        {
            return IO_WR_AGAIN;
        }

        return IO_EXCEPT;
    }

    int CShardMemConn::Recv(Buffer* buf)
    {
        readable = false;
        time_t now = time(NULL);

        if (status == RDCLOSED || status >= HANGUP)
        {
            DEBUG("Conn can not read status : %d", status);

            return IO_NO_READ;
        }

        int len = buf->len - buf->offset;
        int ret = m_recvQueue->Get(buf->data + buf->offset, len);

        buf->offset += ret;

        if (ret == len)
        {
            readable = true;
            lastRead   = now;
            lastActive = now;

            return IO_RD_OK;
        }
        else
        {
            return IO_RD_AGAIN;
        }

        return IO_EXCEPT;
    }

    bool CShardMemConn::Readable()
    {
        return (m_recvQueue && m_recvQueue->Count() > 0);
    }

    bool CShardMemConn::Writeable()
    {
        return (m_sendQueue && m_sendQueue->Count() <= m_sendQueue->Size());
    }

    bool CShardMemConn::IsConnect()
    {
        return (m_shareMem && m_shareMem->GetAttchCnt() >= 2);
    }
}