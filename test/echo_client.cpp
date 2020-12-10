#include "echo_client.h"
#include "net/socket.h"
#include "net/conn.h"

namespace ctm
{
    CEchoClient::CEchoClient() :
        CConnector(),
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
        if (CConnector::Init() == -1)
        {
            return -1;
        }

        m_conn = Connect(ip, port);
        if (m_conn == NULL)
        {
            return -1;
        }

        m_stdin = CreateConn(STDIN_FILENO, EVENT_READ/* | EVENT_EPOLL_ET*/);
        if (m_stdin == NULL)
        {
            ERROR("CreateConn conn failed!");
            return -1;
        }

        return 0;
    }

    void CEchoClient::OnAsynConnTimeOut(CConn* conn)
    {
        DEBUG("On Asyn Conn Time Out");
        m_status = EXCEPT;
    }

    void CEchoClient::OnRead(CConn* conn)
    {
        int ret = 0;
        Buffer buf(4096 * 2);
        if (conn == m_stdin)
        {
            ret = conn->Recv(&buf);

            if (buf.offset > 0)
            {
                m_conn->AsynSend(buf.data, buf.offset);
                m_sendLen +=  buf.offset;
            }

            OnError(conn, ret);
        }
        else if (conn == m_conn)
        {
            ret = conn->Recv(&buf);

            if (buf.offset > 0)
            {
                write(STDOUT_FILENO, buf.data, buf.offset);
                m_recvLen +=  buf.offset;
            }

            OnError(conn, ret);
        }
    }

    void CEchoClient::OnWrite(CConn* conn)
    {
        conn->OnAsynWrite();
        
        if (conn == m_conn)
        {
            if (m_stdin->status == CConn::CLOSED && m_conn->sendCache.size() == 0)
            {
                DEBUG("Send Over len =%d [%s]", m_sendLen, m_conn->ToString().c_str());

                m_conn->CloseWrite();
                OnWriteClose(m_conn);
            }
        }
    }

    void CEchoClient::OnReadClose(CConn* conn)
    {
        DEBUG("OnReadClose:[%s]", conn->ToString().c_str());

        if (conn == m_stdin)
        {
            if (m_conn->sendCache.size() == 0)
            {
                DEBUG("Send Over len =%d [%s]", m_sendLen, m_conn->ToString().c_str());
                m_conn->CloseWrite();
                OnWriteClose(m_conn);
            }
            OnClose(conn);
        }
        else if (conn == m_conn)
        {
            OnClose(conn);
            m_status = EXIT;
        }
    }

    void CEchoClient::OnClose(CConn* conn)
    {
        DEBUG("OnClose:[%s]", conn->ToString().c_str());

        CConnector::OnClose(conn);
        
        if (m_conn == conn)
        {
            m_status = EXIT;
        }
    }
}