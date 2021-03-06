#include "event/event.h"
#include "event/conn_handler.h"
#include "net/socket.h"
#include "ipc/semaphore.h"
#include "ipc/sharememory.h"
#include "connector.h"
#include "net/memconn.h"
#include "common/mem_queue.h"
#include "common/log.h"

#define MAX_CONN_SIZE (1024 * 100)
#define MEM_GAP (32)

namespace ctm
{
    CConnector::CConnector() :
        m_status(CREATE),
        m_log(NULL), 
        m_connPool(NULL),
        m_eventHandler(NULL),
        m_eventMonitor(NULL),
        m_timeOut(1000),
        m_shareMem(NULL),
        m_semaphore(NULL)

    {

    }

    CConnector::~CConnector()
    {
        m_log = NULL;
        DELETE(m_connPool);
        DELETE(m_eventMonitor);
        DELETE(m_eventHandler);
        DELETE(m_shareMem);
        DELETE(m_semaphore);
    }

    int CConnector::Init()
    {
        if (InitBase() == -1)
        {
            return -1;
        }

        m_status = INIT;

        return 0;
    }

    int CConnector::Execute()
    {
        if (m_status == INIT)
        {
            m_status = RUNNING;
        }

        if(m_status == RUNNING)
        {
            if (HandleIOEvent() == -1)
            {
                DEBUG("Handle conns I/O event failed!");
                return -1;
            }
            /*
            if (HandleReadyCConns() == -1)
            {
                DEBUG("Handle already Conns I/O event failed!");
                return -1;
            }
            HandleMemroyConns();
            */
            return 0;
        }
        
        return -1;
    }

    int CConnector::InitBase()
    {
        m_connPool = new CConnPool(MAX_CONN_SIZE);
        if (m_connPool == NULL)
        {
            ERROR("Create conn pool failed!");
            return -1;
        }

        m_eventMonitor = CrateEventMonitor(CEventMonitor::EPOLL);
        if (m_eventMonitor == NULL)
        {
            ERROR("Create epoll event montior failed!");
            return -1;
        }

        if (m_eventMonitor->Init() == -1)
        {
            ERROR("montior init failed!");
            return -1;
        }

        m_eventHandler = new CConnHandler;
        if (m_eventHandler == NULL)
        {
            ERROR("Create event handler failed!");
            return -1;
        }

        return 0;
    }

    void CConnector::OnAccept(CConn* conn)
    {
        HandleAccpet(conn);
    }

    void  CConnector::OnRead(CConn* conn)
    {
    }

    void CConnector::OnWrite(CConn* conn)
    {
        conn->OnAsynWrite();
    }

    void CConnector::OnReadClose(CConn* conn)
    {
        DEBUG("OnReadClose:[%s]", conn->ToString().c_str());

        if (conn->status == CConn::HANGUP)
        {
            OnClose(conn);
        }
    }

    void CConnector::OnWriteClose(CConn* conn)
    {
        DEBUG("OnWriteClose:[%s]", conn->ToString().c_str());

        if (conn->status == CConn::HANGUP)
        {
            OnClose(conn);
        }
    }

    void CConnector::OnClose(CConn* conn)
    {
        Release(conn);
    }

    void CConnector::OnAsynConnOk(CConn* conn)
    {
        conn->GetLocalAddr();
        conn->GetPeerAddr();
        DEBUG("asyn conn ok:[%s]", conn->ToString().c_str());
    }

    void CConnector::OnAsynConnTimeOut(CConn* conn)
    {
        DEBUG("asyn conn time out:[%s]", conn->ToString().c_str());
        OnClose(conn);
    }

    void CConnector::OnHangUp(CConn* conn)
    {
        DEBUG("conn hang up:[%s]", conn->ToString().c_str());
        OnRead(conn);
        if (conn->type == C_SOCK)
        {
            OnWrite(conn);
        }
    }

    void CConnector::OnReady(CConn* conn)
    {
        if ((conn->event.events & EVENT_EPOLL_ET) &&
            m_readyCConnsSet.find(conn) == m_readyCConnsSet.end())
        {
            m_readyCConns.push_back(conn);
            m_readyCConnsSet.insert(conn);
        }  
    }

    void CConnector::OnError(CConn* conn, int error)
    {
        switch (error)
        {
        case IO_RD_OK:
            // OnReady(conn);
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
            // OnWrite(conn);
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
            OnClose(conn);
            break;
        }
    }

    int CConnector::HandleIOEvent()
    {
        int ret = m_eventMonitor->WaitProc(m_timeOut);
        if (ret == -1)
        {
            ERROR("WaitProc failed!");
            return -1;
        }
        return 0;
    }

    int CConnector::HandleReadyCConns()
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

    int CConnector::HandleMemroyConns()
    {
        time_t now = time(NULL);
        CShardMemConn* conn = NULL;
        std::set<CConn*>::iterator it = m_memConnSet.begin();
        while(it != m_memConnSet.end())
        {
            conn = dynamic_cast<CShardMemConn*>(*it++);
            if (conn->status == CConn::CONNING)
            {
                if (conn->IsConnect())
                {
                    conn->status = CConn::ACTIVE;
                    DEBUG("memConn conn OK!");
                }
            }
            else if (conn->status == CConn::ACTIVE)
            {
                if (now - conn->lastRead > 3 && !conn->IsConnect())
                {
                    OnClose(conn);
                    continue;
                }

                if (conn->Writeable())
                {
                    conn->writable = true;
                    OnWrite(conn);
                }

                if (conn->Readable())
                {
                    conn->readable = true;
                    OnRead(conn);
                }
            }
        }

        return 0;
    }

    CConn* CConnector::HandleAccpet(CConn* listenConn)
    {
        if (listenConn->isListen == false)
        {
            ERROR("Conn not Listen!");
            return NULL;
        }

        int fd = 0;
        int err = 0;
        struct sockaddr_in remoteAddr = {0};
        SOCKETLEN_T len = sizeof(remoteAddr);
        while(1)
        {
            fd = accept(listenConn->fd, (struct sockaddr*)&remoteAddr, &len);
            if (fd == SOCKET_INVALID)
            {
                err = errno;
                if (EINTR == err) 
                {
                    continue;
                }
                else if (EAGAIN == err || EWOULDBLOCK == err)
                {
                    ERROR("Accept EAGAIN %d:%s!", err, strerror(err));
                }
                else if (ECONNABORTED == err)
                {
                    ERROR("Accept ECONNABORTED %d:%s!", err, strerror(err));
                }
                else
                {
                    ERROR("Accept other error %d:%s!", err, strerror(err));
                }
                return NULL;
            }

            break;
        }

        SetKeepAlive(fd, 10);
        SetNoDelay(fd);

        CConn* conn = CreateConn(fd, EVENT_READ | EVENT_WRITE | EVENT_PEER_CLOSE /*| EVENT_EPOLL_ET*/ , CConn::ACTIVE, false, AF_INET);
        
        if (conn == NULL)
        {   
            ERROR("CreateConn conn failed!");
            return NULL;
        }

        conn->GetLocalAddr();
        memcpy(&conn->peerAddr, &remoteAddr, len);

        DEBUG("new conn [%s]", conn->ToString().c_str());
        return conn;
    }

    CConn* CConnector::Listen(const string& ip, unsigned int port)
    {
        int fd = ListenSocket(ip.c_str(), port, MAX_CONN_SIZE);
        if (fd == -1)
        {
            ERROR("Listen sock failed!");
            return NULL;
        }

        CConn* conn = CreateConn(fd, EVENT_READ, CConn::ACTIVE, true, AF_INET);
        if (conn == NULL)
        {   
            ERROR("CreateConn conn failed!");
            return NULL;
        }

        return conn;
    }

    CConn* CConnector::Connect(const string& ip, unsigned int port)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd == -1)
        {
            ERROR("socket faield!");
            return NULL;
        }

        CConn* conn = NULL;
        
        int ret = ctm::Connect(fd, ip, port);
        if (ret < 0)
        {
            if (errno != EINPROGRESS)
            {
                close(fd);
                ERROR("Connect faield!");
                return NULL;
            }

            conn = CreateConn(fd, EVENT_READ | EVENT_WRITE | EVENT_PEER_CLOSE /*| EVENT_EPOLL_ET*/, CConn::CONNING, false, AF_INET);
        }
        else
        {
            conn = CreateConn(fd, EVENT_READ | EVENT_WRITE | EVENT_PEER_CLOSE /*| EVENT_EPOLL_ET*/, CConn::ACTIVE, false, AF_INET);
            conn->GetLocalAddr();
            conn->GetPeerAddr();
        }

        return conn;
    }

    CConn* CConnector::AsynConnect(const string& ip, unsigned int port)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd == -1)
        {
            ERROR("socket faield!");
            return NULL;
        }

        SetNonBlock(fd);

        CConn* conn = NULL;
        
        int ret = ctm::Connect(fd, ip, port);
        if (ret < 0)
        {
            if (errno != EINPROGRESS)
            {
                close(fd);
                ERROR("Connect faield!");
                return NULL;
            }

            conn = CreateConn(fd, EVENT_READ | EVENT_WRITE | EVENT_PEER_CLOSE /*| EVENT_EPOLL_ET*/, CConn::CONNING, false, AF_INET);
        }
        else
        {
            conn = CreateConn(fd, EVENT_READ | EVENT_WRITE | EVENT_PEER_CLOSE /*| EVENT_EPOLL_ET*/, CConn::ACTIVE, false, AF_INET);
            conn->GetLocalAddr();
            conn->GetPeerAddr();
        }

        return conn;
    }

    CConn* CConnector::MemroyConn(unsigned int key, unsigned int size, bool bServer)
    {
        CShardMemConn* conn = dynamic_cast<CShardMemConn*>(m_connPool->Create(C_SMEM));
        if (conn == NULL)
        {   
            ERROR("Get conn failed!");
            return NULL;
        }

        if (conn->Open(key, size, bServer) == -1)
        {
            ERROR("open MemroyConn failed!");
            return NULL;
        }

        conn->family = 0;
        conn->type = C_SMEM;
        conn->status = CConn::CONNING;
        conn->isListen = false;
        conn->action = this;
        conn->readable = false;
        conn->writable = false;
        conn->log = m_log;

        m_memConnSet.insert(conn);

        return conn;
    }

    int CConnector::Pipe(CConn* ConnArr[2])
    {
        int pipefd[2] = {0};
        int ret = pipe(pipefd);
        if (ret == -1)
        {
            ERROR("pipe faield %d:%s.", errno, strerror(errno));
            return -1;
        }

        ConnArr[0] = CreateConn(pipefd[0], EVENT_READ | EVENT_PEER_CLOSE /*| EVENT_EPOLL_ET*/);
        ConnArr[1] = CreateConn(pipefd[1], EVENT_WRITE | EVENT_PEER_CLOSE /*| EVENT_EPOLL_ET*/);
        if (ConnArr[0] == NULL || ConnArr[1] == NULL)
        {   
            ERROR("CreateConn conn failed!");
            return -1;
        }

        return 0;
    }

    void CConnector::Release(CConn* conn)
    {
        if (conn->type == C_SMEM)
        {
            CShardMemConn* memConn = dynamic_cast<CShardMemConn*>(conn);

            if (!memConn->m_bServer)
            {
                m_memConnSet.erase(memConn);
                m_connPool->Free(memConn);
                DEBUG("Release memConn client!");
            }
            else
            {
                DEBUG("Release memConn server!");
                memConn->status = CConn::CONNING;
            }
        }
        else
        {
            m_connPool->Free(conn);
        }
    }

    int CConnector::CreateMemConn(unsigned int key, unsigned int ccnt, unsigned int csize)
    {
        m_memConnCnt  = ccnt;
        m_connMemSize = csize;

        unsigned int memSize = 2 * (csize + MEM_GAP) * (ccnt + 1);
        m_shareMem = new CShareMemory(key);
        if (m_shareMem == NULL)
        {
            ERROR("Create share memory failed!");
            return -1;
        }

        if (!m_shareMem->Open(memSize))
        {
            ERROR("Open share memory failed!");
            return -1;
        }

        m_semaphore = new CSemaphore(key, ccnt);
        if (m_semaphore == NULL)
        {
            ERROR("Create semaphore failed!");
            return -1;
        }

        if (!m_semaphore->Open())
        {
            ERROR("Open semaphore failed!");
            return -1;
        }

        return 0;
    }

    CConn* CConnector::CreateConn(int fd, int events, int status, bool listen, int family)
    {
        CConn* conn = m_connPool->Create(C_SOCK);
        if (conn == NULL)
        {   
            close(fd);
            ERROR("Get conn failed!");
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
            ERROR("AddEvent failed!");
            Release(conn);
            return NULL;
        }

        return conn;
    }
}