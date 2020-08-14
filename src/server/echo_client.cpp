#include "echo_client.h"
#include "net/socket.h"
#include "net/conn.h"

namespace ctm
{
    CEchoClient::CEchoClient() :
        CBaseServer(),
        m_stdin(NULL),
        m_conn(NULL),
        m_sendLen(0),
        m_recvLen(0)
    {

    }

    CEchoClient::~CEchoClient()
    {

    }

    int CEchoClient::Init(const string& ip, unsigned int port, CLog* log)
    {
        if (CBaseServer::Init(log) == -1)
        {
            return -1;
        }

        m_conn = Connect(ip, port);
        if (m_conn == NULL)
        {
            return -1;
        }

        m_stdin = CreateConn(STDIN_FILENO, EVENT_READ | EVENT_EPOLL_ET);
        if (m_stdin == NULL)
        {
            CTM_ERROR_LOG(m_log, "CreateConn conn failed!");
            return -1;
        }

        return 0;
    }

    void CEchoClient::OnAsynConnTimeOut(CConn* conn)
    {
        CTM_DEBUG_LOG(m_log, "On Asyn Conn Time Out");
        m_status = EXCEPT;
    }

    void CEchoClient::OnRead(CConn* conn)
    {
        int ret = 0;
        Buffer buf(4096);
        if (conn == m_stdin)
        {
            ret = conn->Recv(&buf);

            if (buf.offset > 0)
            {
                m_conn->AsynSend(buf.data, buf.offset);
                m_sendLen +=  buf.offset;
            }

            switch (ret)
            {
            case IO_RD_OK:
                buf.offset = 0;
                break;
            case IO_RD_AGAIN:
                break;
            case IO_RD_CLOSE:
                { 
                    if (m_conn->sendCache.size() == 0)
                    {
                        CTM_DEBUG_LOG(m_log, "Send Over len =%d [%s]", m_sendLen, m_conn->ToString().c_str());
                        m_conn->CloseWrite();
                        OnWriteClose(m_conn);
                    }
                }
                break;
            case IO_EXCEPT:
                CTM_DEBUG_LOG(m_log, "offset = %d IO_EXCEPT:[%s]", buf.offset, conn->ToString().c_str());
                break;
            }
        }
        else if (conn == m_conn)
        {
            ret = conn->Recv(&buf);

            if (buf.offset > 0)
            {
                write(STDOUT_FILENO, buf.data, buf.offset);
                m_recvLen +=  buf.offset;
            }

            switch (ret)
            {
            case IO_RD_OK:
            case IO_RD_AGAIN:
            case IO_RD_CLOSE:
                break;
            case IO_EXCEPT:
                CTM_DEBUG_LOG(m_log, "offset = %d IO_EXCEPT:[%s]", buf.offset, conn->ToString().c_str());
                m_status = EXIT;
                break;
            }
        }
    }

    void CEchoClient::OnWrite(CConn* conn)
    {
        conn->OnAsynWrite();
        
        if (conn == m_conn)
        {
            if (m_stdin->status == CConn::CLOSED && m_conn->sendCache.size() == 0)
            {
                CTM_DEBUG_LOG(m_log, "Send Over len =%d [%s]", m_sendLen, m_conn->ToString().c_str());

                m_conn->CloseWrite();
                OnWriteClose(m_conn);
            }
        }
    }

    void CEchoClient::OnReadClose(CConn* conn)
    {
        CTM_DEBUG_LOG(m_log, "OnReadClose:[%s]", conn->ToString().c_str());

        if (conn == m_stdin)
        {
            OnClose(conn);
        }
        else if (conn == m_conn)
        {
            OnClose(conn);
            m_status = EXIT;
        }
    }
    
    void CEchoClient::OnException(CConn* conn)
    {
        CTM_DEBUG_LOG(m_log, "OnException:[%s]", conn->ToString().c_str());

        if (m_conn == conn)
        {
            OnClose(conn);
            m_status = EXIT;
        }
        else if (conn == m_stdin)
        {
            OnClose(conn);
            m_status = EXIT;
        }
    }
}