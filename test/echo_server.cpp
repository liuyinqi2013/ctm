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
        if (CConnector::Init() == -1)
        {
            DEBUG("CConnector init failed");
            return -1;
        }

        CConn* listenConn = Listen(ip, port);
        if (listenConn == NULL)
        {
            DEBUG("Listen failed");
            return -1;
        }

        return 0;
    }
        
    void CEchoServer::OnRead(CConn* conn)
    {
        int ret = 0;
        Buffer buf(4096 * 2);

        for (int i = 0; i < 3; i++)
        {
            buf.offset = 0;
            ret = conn->Recv(&buf);
            if (buf.offset > 0) 
            {
                conn->AsynSend(buf.data, buf.offset);
            }

            if (ret != IO_RD_OK)
            {
                break;
            }
        }
        
        OnError(conn, ret);
    }

    void CEchoServer::OnWrite(CConn* conn)
    {
        conn->OnAsynWrite();

        if (conn->status == CConn::RDCLOSED && conn->sendCache.size() == 0)
        {
            DEBUG("XXX OnWrite:[%s]", conn->ToString().c_str());
            OnClose(conn);
        }
    }

    void CEchoServer::OnReadClose(CConn* conn)
    {
        if (conn->sendCache.size() == 0) 
        {
            DEBUG("XXX IO_RD_CLOSE:[%s]", conn->ToString().c_str());
            OnClose(conn);
        }
    }
}