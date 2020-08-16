#include "event/event.h"
#include "event/conn_handler.h"
#include "net/socket.h"
#include "conn_server.h"

#define MAX_CONN_SIZE (1024 * 10)

namespace ctm
{
    CConnServer::CConnServer() :
        m_status(CREATE),
        m_log(NULL), 
        m_connPool(NULL),
        m_eventHandler(NULL),
        m_eventMonitor(NULL)
    {

    }

    CConnServer::~CConnServer()
    {
        m_log = NULL;
        DELETE(m_connPool);
        DELETE(m_eventMonitor);
        DELETE(m_eventHandler);
    }

    int CConnServer::Init(CLog* log)
    {
        m_log = log;

        if (InitBase() == -1)
        {
            return -1;
        }

        m_status = INIT;

        return 0;
    }

    int CConnServer::Execute()
    {
        if (m_status == INIT)
        {
            m_status = RUNNING;
        }

        if(m_status == RUNNING)
        {
            if (HandleIOEvent() != 0)
            {
                CTM_DEBUG_LOG(m_log, "Handle conns I/O event failed!");
                return -1;
            }

            if (HandleReadyCConns() != 0)
            {
                CTM_DEBUG_LOG(m_log, "Handle already Conns I/O event failed!");
                return -1;
            }

            return 0;
        }
        
        return -1;
    }

    int CConnServer::InitBase()
    {
        m_connPool = new CConnPool(MAX_CONN_SIZE);
        if (m_connPool == NULL)
        {
            CTM_ERROR_LOG(m_log, "Create conn pool failed!");
            return -1;
        }

        m_eventMonitor = CrateEventMonitor(CEventMonitor::EPOLL, m_log);
        if (m_eventMonitor == NULL)
        {
            CTM_ERROR_LOG(m_log, "Create epoll event montior failed!");
            return -1;
        }

        if (m_eventMonitor->Init() == -1)
        {
            CTM_ERROR_LOG(m_log, "montior init failed!");
            return -1;
        }

        m_eventHandler = new CConnHandler;
        if (m_eventHandler == NULL)
        {
            CTM_ERROR_LOG(m_log, "Create event handler failed!");
            return -1;
        }

        return 0;
    }

    void CConnServer::OnAccept(CConn* conn)
    {
        HandleAccpet(conn);
    }

    void  CConnServer::OnRead(CConn* conn)
    {
    }

    void CConnServer::OnWrite(CConn* conn)
    {
        conn->OnAsynWrite();
    }

    void CConnServer::OnReadClose(CConn* conn)
    {
        CTM_DEBUG_LOG(m_log, "OnReadClose:[%s]", conn->ToString().c_str());

        if (conn->status == CConn::CLOSED)
        {
            OnClose(conn);
        }
    }

    void CConnServer::OnWriteClose(CConn* conn)
    {
        CTM_DEBUG_LOG(m_log, "OnWriteClose:[%s]", conn->ToString().c_str());

        if (conn->status == CConn::CLOSED)
        {
            OnClose(conn);
        }
    }

    void CConnServer::OnClose(CConn* conn)
    {
        m_connPool->Free(conn);
    }

    void CConnServer::OnAsynConnOk(CConn* conn)
    {
        conn->GetLocalAddr();
        conn->GetPeerAddr();
        CTM_DEBUG_LOG(m_log, "asyn conn ok:[%s]", conn->ToString().c_str());
    }

    void CConnServer::OnAsynConnTimeOut(CConn* conn)
    {
        CTM_DEBUG_LOG(m_log, "asyn conn time out:[%s]", conn->ToString().c_str());
        OnClose(conn);
    }

    void CConnServer::OnHangUp(CConn* conn)
    {
        CTM_DEBUG_LOG(m_log, "conn hang up:[%s]", conn->ToString().c_str());
        OnRead(conn);
        if (conn->type == C_SOCK)
        {
            OnWrite(conn);
        }

        // OnClose(conn);
    }

    void CConnServer::OnReady(CConn* conn)
    {
        if (m_readyCConnsSet.find(conn) == m_readyCConnsSet.end())
        {
            m_readyCConns.push_back(conn);
            m_readyCConnsSet.insert(conn);
        }  
    }

    void CConnServer::OnError(CConn* conn, int error)
    {
        switch (error)
        {
        case IO_RD_OK:
            OnReady(conn);
            break;
        case IO_RD_AGAIN:
            break;
        case IO_NO_READ:
            break;
        case IO_RD_CLOSE:
            conn->CloseRead();
            OnReadClose(conn);
            break;
        case IO_WR_OK:
            OnReady(conn);
            break;
        case IO_WR_AGAIN:
            break;
        case IO_NO_WRITE:
            conn->ClearCache();
            break;
        case IO_WR_CLOSE:
            conn->CloseWrite();
            OnWriteClose(conn);
            break;
        case IO_EXCEPT:
            conn->Close();
            OnClose(conn);
            break;
        }
    }

    int CConnServer::HandleIOEvent()
    {
        if (m_eventMonitor->WaitProc(1) != 0)
        {
            CTM_ERROR_LOG(m_log, "WaitProc failed!");
            return -1;
        }
        return 0;
    }

    int CConnServer::HandleReadyCConns()
    {
        int size = m_readyCConns.size();
        for(int i = 0; i < size; i++)
        {
            CConn* conn = m_readyCConns.front();
            m_readyCConns.pop_front();
            m_readyCConnsSet.erase(conn);

            if (conn->readable)
            {
                conn->action->OnRead(conn);
            }

            if (conn->writable)
            {
                conn->action->OnWrite(conn);
            }
        }

        return 0;
    }

    CConn* CConnServer::HandleAccpet(CConn* listenConn)
    {
        if (listenConn->isListen == false)
        {
            CTM_ERROR_LOG(m_log, "Conn not Listen!");
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
                if (EINTR == err) 
                {
                    continue;
                }
                else if (EAGAIN == err || EWOULDBLOCK == err)
                {
                    CTM_ERROR_LOG(m_log, "Accept EAGAIN %d:%s!", err, strerror(err));
                }
                else if (ECONNABORTED == err)
                {
                    CTM_ERROR_LOG(m_log, "Accept ECONNABORTED %d:%s!", err, strerror(err));
                }
                else
                {
                    CTM_ERROR_LOG(m_log, "Accept other error %d:%s!", err, strerror(err));
                }
                return NULL;
            }

            break;
        }

        SetKeepAlive(fd, 10);

        CConn* newConn = CreateConn(fd, EVENT_READ | EVENT_WRITE | EVENT_EPOLL_ET | EVENT_PEER_CLOSE, CConn::ACTIVE, false, AF_INET);
        if (newConn == NULL)
        {   
            CTM_ERROR_LOG(m_log, "CreateConn conn failed!");
            return NULL;
        }

        newConn->GetLocalAddr();
        memcpy(&newConn->peerAddr, &remoteAddr, len);

        CTM_DEBUG_LOG(m_log, "new conn [%s]", newConn->ToString().c_str());
        return newConn;
    }

    CConn* CConnServer::Listen(const string& ip, unsigned int port)
    {
        int fd = ListenSocket(ip.c_str(), port, MAX_CONN_SIZE);
        if (fd == -1)
        {
            CTM_ERROR_LOG(m_log, "Listen sock failed!");
            return NULL;
        }

        CConn* conn = CreateConn(fd, EVENT_READ, CConn::ACTIVE, true, AF_INET);
        if (conn == NULL)
        {   
            CTM_ERROR_LOG(m_log, "CreateConn conn failed!");
            return NULL;
        }

        return conn;
    }

    CConn* CConnServer::Connect(const string& ip, unsigned int port)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd == -1)
        {
            CTM_ERROR_LOG(m_log, "socket faield!");
            return NULL;
        }

        CConn* conn = NULL;
        
        int ret = ctm::Connect(fd, ip, port);
        if (ret < 0)
        {
            if (errno != EINPROGRESS)
            {
                close(fd);
                CTM_ERROR_LOG(m_log, "Connect faield!");
                return NULL;
            }

            conn = CreateConn(fd, EVENT_READ | EVENT_WRITE | EVENT_EPOLL_ET, CConn::CONNING, false, AF_INET);
        }
        else
        {
            conn = CreateConn(fd, EVENT_READ | EVENT_WRITE | EVENT_EPOLL_ET, CConn::ACTIVE, false, AF_INET);
            conn->GetLocalAddr();
            conn->GetPeerAddr();
        }

        return conn;
    }

    int CConnServer::Pipe(CConn* ConnArr[2])
    {
        int pipefd[2] = {0};
        int ret = pipe(pipefd);
        if (ret == -1)
        {
            CTM_ERROR_LOG(m_log, "pipe faield %d:%s.", errno, strerror(errno));
            return -1;
        }

        ConnArr[0] = CreateConn(pipefd[0], EVENT_READ | EVENT_EPOLL_ET);
        ConnArr[1] = CreateConn(pipefd[1], EVENT_WRITE | EVENT_EPOLL_ET);
        if (ConnArr[0] == NULL || ConnArr[1] == NULL)
        {   
            CTM_ERROR_LOG(m_log, "CreateConn conn failed!");
            return -1;
        }

        return 0;
    }

    CConn* CConnServer::CreateConn(int fd, int events, int status, bool listen, int family)
    {
        CConn* conn = m_connPool->Get(fd);
        if (conn == NULL)
        {   
            close(fd);
            CTM_ERROR_LOG(m_log, "Get conn failed!");
            return NULL;
        }

        SetNonBlock(fd);

        conn->fd = fd;
        conn->family = family;
        conn->type = FileType(fd);
        conn->status = status;
        conn->isListen = listen;
        conn->action = this;
        conn->readable = false;
        conn->writable = false;
        conn->log = m_log;

        conn->event.fd = fd;
        conn->event.data = conn;
        conn->event.active = 0;
        conn->event.handler = m_eventHandler;
        conn->event.monitor = m_eventMonitor;

        int err = m_eventMonitor->AddEvent(&conn->event, events);
        if (err == -1)
        {
            CTM_ERROR_LOG(m_log, "AddEvent failed!");
            m_connPool->Free(conn);
            return NULL;
        }

        return conn;
    }
}