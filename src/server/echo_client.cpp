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
        m_log = log;

        if (InitBase() == -1)
        {
            return -1;
        }

        m_conn = ConnectConn(ip, port);
        if (m_conn == NULL)
        {
            return -1;
        }

        SetNonBlock(STDIN_FILENO);

        m_stdin = m_connPool->Get(STDIN_FILENO);
        m_stdin->fd = STDIN_FILENO;
        m_stdin->type = FileType(STDIN_FILENO);
        m_stdin->status = CConn::ACTIVE;
        m_stdin->isListen = false;
        m_stdin->log = m_log;
        m_stdin->action = this;
        m_stdin->event.fd = STDIN_FILENO;
        m_stdin->event.data = m_stdin;
        m_stdin->event.handler = m_eventHandler;
        m_stdin->event.monitor = m_eventMonitor;

        int err = m_eventMonitor->AddEvent(&m_stdin->event, EVENT_READ | EVENT_EPOLL_ET);
        if (err)
        {
            CTM_DEBUG_LOG(m_log, "AddEvent faield %d:%s!", err, strerror(err));
            m_connPool->Free(m_stdin);
            return -1;
        }

        m_status = INIT;

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
            switch (ret)
            {
            case IO_RD_OK:
                m_conn->AsynSend(buf.data, buf.offset);
                m_sendLen +=  buf.offset;
                buf.offset = 0;
                break;
            case IO_RD_AGAIN:
                { 
                    if (buf.offset > 0) 
                    {
                        m_conn->AsynSend(buf.data, buf.offset);
                        m_sendLen +=  buf.offset;
                    }
                }
                break;
            case IO_RD_CLOSE:
                { 
                    if (buf.offset > 0) 
                    {
                        m_conn->AsynSend(buf.data, buf.offset);
                        m_sendLen +=  buf.offset;
                    }
                    if (m_conn->sendCache.size() == 0)
                    {
                        CTM_DEBUG_LOG(m_log, "Send Over [%s]", m_conn->ToString().c_str());
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
            switch (ret)
            {
            case IO_RD_OK:
                write(STDOUT_FILENO, buf.data, buf.offset);;
                m_recvLen +=  buf.offset;
                buf.offset = 0;
                break;
            case IO_RD_AGAIN:
            case IO_RD_CLOSE:
                { 
                    if (buf.offset > 0) 
                    {
                        write(STDOUT_FILENO, buf.data, buf.offset);
                        m_recvLen +=  buf.offset;
                    } 
                }
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
                CTM_DEBUG_LOG(m_log, "Send Over [%s]", conn->ToString().c_str());
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
            m_connPool->Free(conn);
        }
        else if (conn == m_conn)
        {
            m_connPool->Free(conn);
            m_status = EXIT;
        }
    }

    void CEchoClient::OnException(CConn* conn)
    {
        CTM_DEBUG_LOG(m_log, "OnException:[%s]", conn->ToString().c_str());

        if (m_conn == conn)
        {
            m_connPool->Free(conn);
            m_status = EXIT;
        }
        else if (conn == m_stdin)
        {
            if (m_conn->sendCache.size() == 0)
            {
                CTM_DEBUG_LOG(m_log, "Send Over [%s]", conn->ToString().c_str());
                m_conn->CloseWrite();
                OnWriteClose(m_conn);
            }

            m_connPool->Free(conn);
        }
    }
}