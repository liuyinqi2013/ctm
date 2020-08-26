#ifndef CTM_NET_MEM_CONN_H__
#define CTM_NET_MEM_CONN_H__

#include "conn.h"

namespace ctm
{
    class CSemaphore;
    class CMemoryQueue;
    class CShareMemory;

    class CShardMemConn : public CConn
    {
    public:
        CShardMemConn();

        virtual ~CShardMemConn();

        int Open(unsigned int key, unsigned int size, bool bServer);

        virtual void Reset();
        virtual void Close();
        virtual void CloseRead();
        virtual void CloseWrite();

        virtual int Send(Buffer* buf);
        virtual int Recv(Buffer* buf);

        bool Readable();
        bool Writeable();
        
        bool IsConnect();
    
    public:
        bool m_bServer;
        CMemoryQueue*  m_sendQueue;
        CMemoryQueue*  m_recvQueue;
        CShareMemory*  m_shareMem;
        CSemaphore*    m_semaphore;
    };
}

#endif