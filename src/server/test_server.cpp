#include "test_server.h"
#include "net/socket.h"
#include "net/conn.h"

namespace ctm
{
    CTestServer::CTestServer()
    {

    }

    CTestServer::~CTestServer()
    {

    }

    int CTestServer::Init(CLog* log)
    {
        if (CBaseServer::Init(log) == -1)
        {
            return -1;
        }

        int ret = Pipe(m_pipeConn);
        if (ret == -1)
        {
            return -1;
        }
        
        return 0;
    }
        
    void CTestServer::OnRead(CConn* conn)
    {
        CTM_DEBUG_LOG(m_log, "OnRead:[%s]", conn->ToString().c_str());

        Buffer buf(1024);
        int ret = conn->Recv(&buf);
        CTM_DEBUG_LOG(m_log, "Read ret:%d len:%d ", ret, buf.offset);
    }

    void CTestServer::OnWrite(CConn* conn)
    {
        CTM_DEBUG_LOG(m_log, "OnWrite:[%s]", conn->ToString().c_str());
        Buffer buf(4096);
        int ret = m_pipeConn[1]->Send(&buf);
        CTM_DEBUG_LOG(m_log, "Write ret:%d len:%d ", ret, buf.offset);
        m_pipeConn[1]->Close();
        //m_pipeConn[1]->event.monitor->DelEvent(&m_pipeConn[1]->event, EVENT_WRITE);
    }

    void CTestServer::OnException(CConn* conn)
    {
        m_status = EXIT;
        OnClose(conn);
    }
}