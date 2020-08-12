#include "event/event.h"
#include "event/conn_handler.h"
#include "net/socket.h"
#include "base_server.h"

#define MAX_CONN_SIZE (1024 * 10)

namespace ctm
{
    CBaseServer::CBaseServer() :
        m_status(CREATE),
        m_log(NULL), 
        m_connPool(NULL),
        m_eventHandler(NULL),
        m_eventMonitor(NULL)
    {

    }

    CBaseServer::~CBaseServer()
    {
        m_log = NULL;
        DELETE(m_connPool);
        DELETE(m_eventMonitor);
        DELETE(m_eventHandler);
    }

    int  CBaseServer::Init(const string& ip, unsigned int port, CLog* log)
    {
        m_log = log;

        if (InitBase() == -1)
        {
            return -1;
        }

        if (ListenConn(ip, port) == NULL)
        {
            return -1;
        }

        m_status = INIT;

        return 0;
    }

    int CBaseServer::RunProc()
    {
        if (m_status == INIT)
        {
            m_status = RUNNING;
        }

        while(m_status == RUNNING)
        {
            if (m_eventMonitor->WaitProc(100) != 0)
            {
                CTM_DEBUG_LOG(m_log, "WaitProc failed!");
                m_status = EXCEPT;
            }
        }
        
        return 0;
    }

    int CBaseServer::InitBase()
    {
        m_connPool = new CConnPool(MAX_CONN_SIZE);
        if (m_connPool == NULL)
        {
            CTM_DEBUG_LOG(m_log, "Create conn pool failed!");
            return -1;
        }

        m_eventMonitor = CrateEventMonitor(CEventMonitor::EPOLL, m_log);
        if (m_eventMonitor == NULL)
        {
            CTM_DEBUG_LOG(m_log, "Create epoll event montior failed!");
            return -1;
        }

        if (m_eventMonitor->Init() == -1)
        {
            CTM_DEBUG_LOG(m_log, "montior init failed!");
            return -1;
        }

        m_eventHandler = new CConnHandler;
        if (m_eventHandler == NULL)
        {
            CTM_DEBUG_LOG(m_log, "Create event handler failed!");
            return -1;
        }

        return 0;
    }

    void CBaseServer::OnAccept(CConn* conn)
    {
        HandleAccpet(conn);
    }

    void  CBaseServer::OnRead(CConn* conn)
    {
    }

    void CBaseServer::OnWrite(CConn* conn)
    {
        conn->OnAsynWrite();
    }

    void CBaseServer::OnReadClose(CConn* conn)
    {
        CTM_DEBUG_LOG(m_log, "OnReadClose:[%s]", conn->ToString().c_str());

        if (conn->status >= CConn::CLOSED)
        {
            m_connPool->Free(conn);
        }
    }

    void CBaseServer::OnWriteClose(CConn* conn)
    {
        CTM_DEBUG_LOG(m_log, "OnWriteClose:[%s]", conn->ToString().c_str());

        if (conn->status >= CConn::CLOSED)
        {
            m_connPool->Free(conn);
        }
    }

    void CBaseServer::OnClose(CConn* conn)
    {

    }

    void CBaseServer::OnAsynConnOk(CConn* conn)
    {
        CTM_DEBUG_LOG(m_log, "asyn conn ok:[%s]", conn->ToString().c_str());
        conn->GetPeerAddr();
    }

    void CBaseServer::OnAsynConnTimeOut(CConn* conn)
    {
        CTM_DEBUG_LOG(m_log, "asyn conn time out:[%s]", conn->ToString().c_str());
        m_connPool->Free(conn);
    }

    void  CBaseServer::OnException(CConn* conn)
    {
        CTM_DEBUG_LOG(m_log, "conn exception:[%s]", conn->ToString().c_str());
        m_connPool->Free(conn);
    }

    CConn* CBaseServer::HandleAccpet(CConn* listenConn)
    {
        if (listenConn->isListen == false)
        {
            CTM_DEBUG_LOG(m_log, "Conn not Listen!");
            return NULL;
        }

        int fd = 0;
        int err = 0;
        struct sockaddr_in remoteAddr = {0};
        SOCKETLEN_T len = sizeof(remoteAddr);
        while(1)
        {
            fd = accept(listenConn->fd, (struct sockaddr*)&remoteAddr,&len);
            if (fd == SOCKET_INVALID)
            {
                err = errno;
                if (err == EINTR) continue;

                CTM_DEBUG_LOG(m_log, "Accept faield %d:%s!", err, strerror(err));
                return NULL;
            }
            break;
        }

        SetNonBlock(fd);

        CConn* newConn = m_connPool->Get(fd);
        if (newConn == NULL)
        {   
            close(fd);
            CTM_DEBUG_LOG(m_log, "Get conn failed!");
            return NULL;
        }

        newConn->fd = fd;
        newConn->type = FileType(fd);
        newConn->family = AF_INET;
        newConn->status = CConn::ACTIVE;
        newConn->isListen = false;
        newConn->action = this;

        newConn->event.fd = fd;
        newConn->event.data = newConn;
        newConn->event.active = 0;
        newConn->event.handler = m_eventHandler;
        newConn->event.monitor = m_eventMonitor;

        newConn->GetPeerAddr();
        memcpy(&newConn->localAddr, &remoteAddr, len);

        err = m_eventMonitor->AddConn(newConn);
        if (err == -1)
        {
            CTM_DEBUG_LOG(m_log, "AddEvent faield %d:%s!", err, strerror(err));
            m_connPool->Free(newConn);
            return NULL;
        }

        CTM_DEBUG_LOG(m_log, "new conn [%s]", newConn->ToString().c_str());
        return newConn;
    }

    CConn* CBaseServer::ListenConn(const string& ip, unsigned int port)
    {
        int fd = ListenSocket(ip.c_str(), port, MAX_CONN_SIZE);
        if (fd == -1)
        {
            CTM_DEBUG_LOG(m_log, "Listen sock failed!");
            return NULL;
        }

        CConn* conn = m_connPool->Get(fd);
        if (conn == NULL)
        {   
            close(fd);
            CTM_DEBUG_LOG(m_log, "Get conn failed!");
            return NULL;
        }

        conn->fd = fd;
        conn->family = AF_INET;
        conn->type = FileType(fd);
        conn->status = CConn::ACTIVE;
        conn->isListen = true;
        conn->action = this;

        conn->event.fd = fd;
        conn->event.data = conn;
        conn->event.active = 0;
        conn->event.handler = m_eventHandler;
        conn->event.monitor = m_eventMonitor;

        int err = m_eventMonitor->AddEvent(&conn->event, EVENT_READ);
        if (err == -1)
        {
            m_connPool->Free(conn);
            return NULL;
        }

        return conn;
    }

    CConn* CBaseServer::ConnectConn(const string& ip, unsigned int port)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd == -1)
        {
            CTM_DEBUG_LOG(m_log, "socket faield!");
            return NULL;
        }

        SetNonBlock(fd);

        CConn* conn = m_connPool->Get(fd);
        if (conn == NULL)
        {   
            close(fd);
            CTM_DEBUG_LOG(m_log, "Get conn failed!");
            return NULL;
        }

        conn->fd = fd;
        conn->family = AF_INET;
        conn->type = FileType(fd);
        conn->isListen = false;
        conn->action = this;

        conn->event.fd = fd;
        conn->event.data = conn;
        conn->event.active = 0;
        conn->event.handler = m_eventHandler;
        conn->event.monitor = m_eventMonitor;

        int err = 0;
        int ret = ctm::Connect(fd, ip, port);
        if (ret < 0)
        {
            if (errno != EINPROGRESS)
            {
                m_connPool->Free(conn);
                CTM_DEBUG_LOG(m_log, "Connect faield!");
                return NULL;
            }

            conn->status = CConn::CONNING;
            err = m_eventMonitor->AddEvent(&conn->event, EVENT_READ | EVENT_WRITE | EVENT_EPOLL_ET);
        }
        else
        {
            conn->status = CConn::ACTIVE;
            err = m_eventMonitor->AddEvent(&conn->event, EVENT_READ | EVENT_EPOLL_ET);
            conn->GetPeerAddr();
        }

        conn->GetLocalAddr();

        if (err == -1)
        {
            m_connPool->Free(conn);
            return NULL;
        }

        return conn;
    }

    /*
    int CBaseServer::Read(CConn* conn, char* buf, int len)
    {
        int err = 0;
        int retlen = 0;
        int offset = 0;

        while(1)
        {
            retlen = read(conn->fd, buf + offset, len - offset);

            if (retlen < 0)
            {
                err = errno;
                conn->error = err;

                if (EINTR == err) { 
                    continue;
                }
                else if (EAGAIN == err || EWOULDBLOCK == err) {
                    break;
                }
                else {
                    CTM_DEBUG_LOG(m_log, "read faield %d:%s!", err, strerror(err));
                    conn->status = CConn::EXCEPT;
                    conn->action->OnException(conn);
                    conn->event.monitor->DelConn(conn);
                    return -1;
                }
            }
            else if (retlen == 0)
            {
                if (conn->status == CConn::ACTIVE)
                {
                    conn->status = CConn::RDCLOSED;
                    conn->event.monitor->DelEvent(&conn->event, EVENT_READ);
                }
                else
                {
                    conn->status = CConn::CLOSED;
                    conn->event.monitor->DelConn(conn);
                }

                conn->action->OnReadClose(conn);
                break;
            }
            else
            {
                offset += retlen;
                if (offset < len) {
                    continue;
                }
                else {
                    conn->event.monitor->AddEvent(&conn->event, EVENT_READ);
                    break;
                }
            }
        }
        return 0;
    }
    */

}