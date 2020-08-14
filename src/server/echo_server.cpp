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

    int CEchoServer::Init(const string& ip, unsigned int port, CLog* log)
    {
        if (CBaseServer::Init(log) == -1)
        {
            CTM_DEBUG_LOG(m_log, "CBaseServer init failed");
            return -1;
        }

        CConn* listenConn = Listen(ip, port);
        if (listenConn == NULL)
        {
            CTM_DEBUG_LOG(m_log, "Listen failed");
            return -1;
        }

        return 0;
    }
        
    void CEchoServer::OnRead(CConn* conn)
    {
        Buffer buf(4096);
        int ret = conn->Recv(&buf);
        if (buf.offset > 0) 
        {
            conn->AsynSend(buf.data, buf.offset);
        }
             
        switch (ret)
        {
        case IO_RD_OK:
            break;
        case IO_RD_AGAIN:
            break;
        case IO_RD_CLOSE:
            { 
                if (conn->sendCache.size() == 0) 
                {
                    CTM_DEBUG_LOG(m_log, "XXX IO_RD_CLOSE:[%s]", conn->ToString().c_str());
                    OnClose(conn);
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
            CTM_DEBUG_LOG(m_log, "XXX OnWrite:[%s]", conn->ToString().c_str());
            OnClose(conn);
        }
    }

    void CEchoServer::OnException(CConn* conn)
    {
        if (conn->isListen)
        {
            m_status = EXIT;
        }
        OnClose(conn);
    }
}