#include "echo_server.h"
#include "net/socket.h"
#include "net/conn.h"

namespace ctm
{
    CEchoServer::CEchoServer()
    {

    }

    CEchoServer::~CEchoServer()
    {

    }
        
    void CEchoServer::OnRead(CConn* conn)
    {
        Buffer buf(4096);
        int ret = conn->Recv(&buf);
        switch (ret)
        {
        case IO_RD_OK:
            conn->AsynSend(buf.data, buf.offset);
            buf.offset = 0;
            break;
        case IO_RD_AGAIN:
            { 
                if (buf.offset > 0) conn->AsynSend(buf.data, buf.offset); 
            }
            break;
        case IO_RD_CLOSE:
            { 
                if (buf.offset > 0)
                {
                    conn->AsynSend(buf.data, buf.offset);
                }
                if (conn->sendCache.size() == 0) 
                {
                    m_connPool->Free(conn);
                    CTM_DEBUG_LOG(m_log, "XXX IO_RD_CLOSE:[%s]", conn->ToString().c_str());
                }
            }
            break;
        case IO_EXCEPT:
            CTM_DEBUG_LOG(m_log, "offset = %d IO_EXCEPT:[%s]", buf.offset, conn->ToString().c_str());
            break;
        }
    }

    void CEchoServer::OnWrite(CConn* conn)
    {
        conn->OnAsynWrite();

        if (conn->status == CConn::RDCLOSED && conn->sendCache.size() == 0)
        {
            m_connPool->Free(conn);
            CTM_DEBUG_LOG(m_log, "XXX OnWrite:[%s]", conn->ToString().c_str());
        }
    }

    void CEchoServer::OnException(CConn* conn)
    {
        if (conn->isListen)
        {
            m_status = EXIT;
        }
        m_connPool->Free(conn);
    }
}