#include "tcpmod.h"
#include <sys/epoll.h>
#include <errno.h>
#include <assert.h>
#include "common/log.h"
#include "common/lock.h"
#include "thread/mutex.h"

#ifndef BUF_LEN
    #define BUF_LEN          4096
    #define MAX_WAIT_EVENTS  1024
    #define MAX_EPOLL_FDS    1024
#endif

namespace ctm
{
    CTcpMod::CTcpMod(CTcpInterface* tcpInterface) :
        m_epollFd(-1),
        m_tcpInterface(tcpInterface),
        m_mutex(NULL)
    {
    }

    CTcpMod::~CTcpMod()
    {

    }

    int CTcpMod::Init()
    {
        m_epollFd = epoll_create(MAX_EPOLL_FDS);
		if (-1 == m_epollFd)
		{
			ERROR_LOG("epoll_create1 failed");
			return -1;
		}
        m_mutex = new CMutex;
        return 0;
    }

    int CTcpMod::UnInit()
    {
        Stop();
        close(m_epollFd);
        return 0;
    }

    int CTcpMod::OnRunning()
    {
        Start();
        Detach();
        return 0;
    }

    int CTcpMod::Connect(const char* ip, int port)
    {
        SOCKET_T connFd = socket(AF_INET, SOCK_STREAM, 0);
        if (connFd == -1)
        {
            ERROR_LOG("socket failed");
            return -1;
        }

        Conn* conn = NULL;
        ClearSockError(connFd);
        SetNonBlock(connFd);
        int ret = ctm::Connect(connFd, ip, port);
        if (ret < 0)
        {
            if (errno != EINPROGRESS)
            {
                ERROR_LOG("Connect failed ip:%s, port:%d", ip, port);
                CloseSocket(connFd);
                return -1;
            }
            conn = new Conn(connFd, port, ip, Conn::Connecting);
            AddEpollEvent(conn, EPOLLIN | EPOLLOUT);
        }
        else
        {
            SetKeepAlive(connFd, 10);
            conn = new Conn(connFd, port, ip, Conn::Connected);
            AddEpollEvent(conn, EPOLLIN);
        }
        AddConn(conn);
        return 0;
    }

    int CTcpMod::Listen(const char* ip, int port)
    {
        SOCKET_T acceptFd = ListenSocket(ip, port);
        if (acceptFd == SOCKET_INVALID)
        {
            ERROR_LOG("ListenSocket failed");
            return -1;
        }
        SetNonBlock(acceptFd);
        Conn* conn = new Conn(acceptFd, port, ip, Conn::ConnectListen);
        AddEpollEvent(conn, EPOLLIN);
        AddConn(conn);
        return 0;
    }

    void CTcpMod::Send(Conn* conn, char* data, int len)
    {

    }

    bool CTcpMod::IsValidConn(const Conn* conn)
    {
	return true;
    }

    void CTcpMod::SetInterface(CTcpInterface* tcpInterface)
    {

    }

    void CTcpMod::Close(Conn* conn)
    {
        assert(conn != NULL);
        close(conn->fd);
        DelEpollEvent(conn);
        DelConn(conn);
        delete conn;
    }

    void CTcpMod::CloseRead(Conn* conn)
    {
        assert(conn != NULL);
        shutdown(conn->fd, 0);
        if (conn->status == Conn::ConnectWriteClose)
        {
            conn->status = Conn::ConnectClose;
        }
        else
        {
            conn->status = Conn::ConnectReadClose;
        }
        ConnOptNotify(conn, CTcpInterface::CONNECT_READ_CLOSE);
    }

    void CTcpMod::CloseWrite(Conn* conn)
    {
        assert(conn != NULL);
        shutdown(conn->fd, 1);
        if (conn->status == Conn::ConnectReadClose)
        {
            conn->status = Conn::ConnectClose;
        }
        else
        {
            conn->status = Conn::ConnectWriteClose;
        }
        ConnOptNotify(conn, CTcpInterface::CONNECT_WRITE_CLOSE);
    }

    int CTcpMod::Run()
    {
        int i = 0;
		int nfds = 0;
		struct epoll_event events[MAX_WAIT_EVENTS];
        struct Conn* conn = NULL;

		while(1)
		{
			nfds = epoll_wait(m_epollFd, events, MAX_WAIT_EVENTS, -1);
			if (nfds == -1) 
			{
                if (EINTR == errno) continue;
				ERROR_LOG("epoll_wait failed");
				return -1;
			}
			
			for (i = 0; i < nfds; ++i)
			{
                conn = (Conn*)(events[i].data.ptr);
                if (conn->status == Conn::ConnectListen && (events[i].events & EPOLLIN))
                {
                    Accept(conn);
                }
				else if (conn->status == Conn::Connecting)
				{
                    if ((events[i].events & EPOLLIN) && (events[i].events & EPOLLOUT)) 
                    {
                        conn->status = Conn::ConnectError;
                        ConnOptNotify(conn, CTcpInterface::CONNECT_FAIL);
                        Close(conn);
                    }
                    else if (events[i].events & EPOLLIN) 
                    {
                        conn->status = Conn::Connected;
                        ConnOptNotify(conn, CTcpInterface::CONNECT_OK);
                        UpdateEpollEvent(conn, EPOLLIN);
                    }
				}
				else if (conn->status == Conn::Connected)
				{
                    if (events[i].events & EPOLLIN)
                        Read(conn);
                    //if (events[i].events & EPOLLOUT)
				}
			}	
		}

		return 0;
    }

    int CTcpMod::AddConn(Conn* conn)
    {
        ConnMap::iterator it =  m_connMap.find(conn->fd);
        if (it == m_connMap.end())
        {
            m_connMap[conn->fd] = conn;
        }

        return 0;
    }

    int CTcpMod::DelConn(Conn* conn)
    {
        ConnMap::iterator it =  m_connMap.find(conn->fd);
        if (it != m_connMap.end())
        {
            m_connMap.erase(it);
        }

        return 0;
    }

    int CTcpMod::AddEpollEvent(Conn* conn, uint32_t events)
    {
        struct epoll_event event = {0}; 
		event.events  = events;
		//event.data.fd = conn->fd;
        event.data.ptr = (void*)conn;
	
		int iRet = epoll_ctl(m_epollFd, EPOLL_CTL_ADD, conn->fd, &event);
		if (iRet != 0)
		{
			ERROR_LOG("epoll_ctl add Conn:{%s}", conn->ToString().c_str());
			return -1;
		}

        return 0;
    }

    int CTcpMod::DelEpollEvent(Conn* conn)
    {
		int iRet = epoll_ctl(m_epollFd, EPOLL_CTL_DEL, conn->fd, NULL);
		if (iRet != 0)
		{
			ERROR_LOG("epoll_ctl del Conn:{%s}", conn->ToString().c_str());
			return -1;
        }

        return 0;
    }

    int CTcpMod::UpdateEpollEvent(Conn* conn, uint32_t events)
    {
        struct epoll_event event = {0}; 
		event.events  = events;
		//event.data.fd = conn->fd;
        event.data.ptr = (void*)conn;
	
		int iRet = epoll_ctl(m_epollFd, EPOLL_CTL_MOD, conn->fd, &event);
		if (iRet != 0)
		{
			ERROR_LOG("epoll_ctl mod Conn:{%s}", conn->ToString().c_str());
			return -1;
		}

        return 0;
    }

    int CTcpMod::Read(Conn* conn)
    {
        int len =0;
        char buf[BUF_LEN] = {0};
        while (1)
        {
            len = read(conn->fd, buf, sizeof(buf));
            if (len < 0)
            {
                int errnum = errno;
                if (errnum == EINTR) continue;
                if (NotFatalError(errno)) return 0;
                ConnOptNotify(conn, CTcpInterface::CONNECT_CLOSE);
                Close(conn);
                ERROR_LOG("errcode = %d, errmsg = %s", errnum, strerror(errnum));
                return -1;
            }
            else if (len == 0)
            {
                ConnOptNotify(conn, CTcpInterface::CONNECT_READ_CLOSE);
                CloseRead(conn);
                return -1;
            }

            if (m_tcpInterface) m_tcpInterface->OnRecv(conn, buf, len);
        }
        return 0;
    }

    int CTcpMod::Accept(Conn* conn)
    {
        int clientPort;
        string clientIp;
	
        SOCKET_T clientfd = ctm::Accept(conn->fd, clientIp, clientPort);
        DEBUG_LOG("clientfd fd = %d", clientfd);
        if (clientfd == SOCKET_INVALID)
        {
            ERROR_LOG("errcode = %d, errmsg = %s!", errno, strerror(errno));
            if (NotFatalError(errno)) return 0;
            return -1;
        }

        SetNonBlock(clientfd);
        SetKeepAlive(clientfd, 10);

        conn = new Conn(clientfd, clientPort, clientIp, Conn::Connected);
        assert(conn != NULL);
        AddConn(conn);
        AddEpollEvent(conn, EPOLLIN);
        ConnOptNotify(conn, CTcpInterface::CONNECT_NEW);

        return 0;
    }

    void CTcpMod::ConnOptNotify(const Conn* conn, int opt)
    {
        if (m_tcpInterface) m_tcpInterface->OnConnectOpt(conn, opt);
    }

}
