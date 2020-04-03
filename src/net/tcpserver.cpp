#include "tcpserver.h"
#include <sys/epoll.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>
#include "common/log.h"
#include "common/lock.h"
#include "thread/mutex.h"

#define BUF_LEN          4096
#define MAX_WAIT_EVENTS  100
#define MAX_EPOLL_FDS    1024

namespace ctm
{
    CTcpSend::CTcpSend(CTcpServer* tcpserver) :
        m_epollFd(-1),
        m_tcpServr(tcpserver)
    {

    }

    CTcpSend::~CTcpSend()
    {

    }

    int CTcpSend::Init()
    {
        return 0;
    }

    int CTcpSend::UnInit()
    {
        Stop();
        m_queue.Clear();
        return 0;
    }

    int CTcpSend::OnRunning()
    {
        Start();
        Detach();
        return 0;
    }

    int CTcpSend::Run()
    {
        while(1)
        {
            shared_ptr<CNetDataMessage> message = dynamic_pointer_cast<CNetDataMessage>(m_queue.GetFront());
            if (message.get() == NULL)
            {
                continue;
            }

            if (m_tcpServr && !m_tcpServr->IsValidConn(message->m_conn))
            {
                m_queue.PopFront();
                continue;
            }

            if (message->m_buf->offset == 0)
            {
                if (SendPacketSize(message->m_conn.fd, message->m_buf->len) != 0)
                {
                    m_queue.PopFront();
                    continue;
                }
            }

            if (SendPacketData(message->m_conn.fd, *message->m_buf) != 0)
            {
                m_queue.PopFront();
                continue;
            }

            if (IsCompletePack(*message->m_buf))
            {
                m_queue.PopFront();

            }
        }
        return 0;
    }

    CTcpServer::CTcpServer(const string &ip, unsigned int port) :
        m_ip(ip),
        m_port(port),
        m_epollFd(-1),
        m_sendModule(NULL),
        m_mutex(NULL)
    {
    }

    CTcpServer::~CTcpServer()
    {

    }

    int CTcpServer::Init()
    {
        m_epollFd = epoll_create(MAX_EPOLL_FDS);
		if (-1 == m_epollFd)
		{
			ERROR_LOG("epoll_create1 failed");
			return -1;
		}

        m_acceptFd = ListenSocket(m_ip.c_str(), m_port);
        if (m_acceptFd == SOCKET_INVALID)
        {
            ERROR_LOG("ListenSocket failed");
            return -1;
        }

        SetNonBlock(m_acceptFd);

        struct epoll_event event = {0};
		event.events  = EPOLLIN;
		event.data.fd = m_acceptFd;
	
		int iRet = epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_acceptFd, &event);
		if (iRet != 0)
		{
			ERROR_LOG("epoll_ctl failed");
			return -1;
		}

        m_sendModule = new CTcpSend(this);
        m_sendModule->Init();
        m_mutex = new CMutex;

        return 0;
    }

    int CTcpServer::UnInit()
    {
        if (m_sendModule)
        {
            m_sendModule->UnInit();
            DELETE(m_sendModule);
        }
        Stop();
        return 0;
    }

    int CTcpServer::OnRunning()
    {
        m_sendModule->OnRunning();
        Start();
        Detach();
        return 0;
    }

    void CTcpServer::SendData(const Conn& conn, char* data, int len)
    {
        if (!IsValidNetLen(len)) return;
        shared_ptr<CNetDataMessage> message = make_shared<CNetDataMessage>();
        message->m_conn = conn;
        message->m_buf = new RecvBuf(len);
        memcpy(message->m_buf->data, data, len);
        m_sendModule->SendData(message);
    }

    bool CTcpServer::IsValidConn(const Conn& conn)
    {
        CLockOwner owner(*m_mutex);
        ConnMap::iterator it = m_connMap.find(conn.fd);
        if (it != m_connMap.end())
        {
            return bool(*it->second == conn);
        }
        return false;
    }

    int CTcpServer::Run()
    {
        int i = 0;
		int nfds = 0;
		struct epoll_event events[MAX_WAIT_EVENTS];
		string clientIp;
		int clientPort = 0;
		Conn* conn = NULL;
		
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
				if (events[i].data.fd == m_acceptFd)
				{
					SOCKET_T clientSock = Accept(m_acceptFd, clientIp, clientPort);
					DEBUG_LOG("clientSock fd = %d", clientSock);
					if (clientSock == SOCKET_INVALID)
					{
						ERROR_LOG("errcode = %d, errmsg = %s!", errno, strerror(errno));
						if (NotFatalError(errno))
							continue;
						return -1;
					}

                    SetNonBlock(clientSock);
                    SetKeepAlive(clientSock, 10);

					conn = new Conn(clientSock, clientPort, clientIp);
                    assert(conn != NULL);
					AddConn(conn);
                    AddEpollEvent(clientSock);	
				}
				else if (events[i].events & EPOLLRDHUP)
				{
                    DeleteClinetConn(events[i].data.fd);
				}
				else if (events[i].events & EPOLLERR)
				{
                    DeleteClinetConn(events[i].data.fd);
				}	
				else if (events[i].events & EPOLLIN)
				{
                    Read(events[i].data.fd);
				}
			}	
		}

		return 0;
    }

    bool CTcpServer::AddConn(Conn *conn)
    {
        CLockOwner owner(*m_mutex);
        ConnMap::iterator it = m_connMap.find(conn->fd);
        if (it != m_connMap.end())
        {
            ConnOptNotify(*it->second, CNetConnMessage::DISCONNECT);
            delete it->second;
        }
        m_connMap[conn->fd] = conn;
        ConnOptNotify(*conn, CNetConnMessage::CONNECT_OK);

        return true;
    }

    void CTcpServer::AddEpollEvent(SOCKET_T fd)
    {
        struct epoll_event event = {0};
		event.events  = EPOLLIN;
		event.data.fd = fd;
		int iRet = epoll_ctl(m_epollFd, EPOLL_CTL_ADD, fd, &event);
		if (iRet != 0)
		{
			ERROR_LOG("epoll_ctl failed");
		}
    }

    void CTcpServer::DeleteClinetConn(SOCKET_T fd)
    {
        m_contextCache.Remove(fd);
        close(fd);
        DelEpollEvent(fd);
        DelConn(fd);
    }

    void CTcpServer::ConnOptNotify(const Conn& conn, int opt)
    {
        shared_ptr<CNetConnMessage> message = make_shared<CNetConnMessage>();
        message->m_conn = conn;
        message->m_opt = opt;
        if (m_outMessageQueue)
        {
            DEBUG_LOG("Conn %s:%d opt=%d", conn.ip.c_str(), conn.port, opt);
            m_outMessageQueue->PushBack(message);
        }
    }

    void CTcpServer::DelConn(SOCKET_T fd)
    {
        CLockOwner owner(*m_mutex);
        ConnMap::iterator it = m_connMap.find(fd);
        if (it != m_connMap.end())
        {
            ConnOptNotify(*it->second, CNetConnMessage::DISCONNECT);
            delete it->second;
            m_connMap.erase(it);
        }
    }

    void CTcpServer::DelEpollEvent(SOCKET_T fd)
    {
        struct epoll_event event = {0};
		event.events  = EPOLLIN;
		event.data.fd = fd;
		int iRet = epoll_ctl(m_epollFd, EPOLL_CTL_DEL, fd, &event);
		if (iRet != 0){
        }
    }

    int CTcpServer::Read(SOCKET_T fd)
    {
        RecvBuf* buf = m_contextCache.GetRecvBuf(fd);
        if (buf == NULL)
        {
            int ret = ReadPacketSize(fd);
            if (ret == -1 || !IsValidNetLen(ret)) 
            {
                //ERROR_LOG("Read size : %d", ret);
                DeleteClinetConn(fd);
                return -1;
            }

            buf = new RecvBuf(ret);
            ret = ReadPacketData(fd, *buf);
            if (ret == -1) 
            {
                DELETE(buf);
                DeleteClinetConn(fd);
                return -1;
            }

            if (IsCompletePack(*buf))
            {
                shared_ptr<CNetDataMessage> message = make_shared<CNetDataMessage>();
                message->m_conn = *m_connMap[fd];
                message->m_buf = buf;
                if (m_outMessageQueue) m_outMessageQueue->PushBack(message);
            }
            else
            {
                m_contextCache.PutRecvBuf(fd, buf);
            }
        }
        else
        {
            int ret = ReadPacketData(fd, *buf);
            if (ret == -1) 
            {
                DELETE(buf);
                DeleteClinetConn(fd);
                return -1;
            }

            if (IsCompletePack(*buf))
            {
                m_contextCache.Remove(fd);
                shared_ptr<CNetDataMessage> message = make_shared<CNetDataMessage>();
                message->m_conn = *m_connMap[fd];
                message->m_buf = buf;
                if (m_outMessageQueue) m_outMessageQueue->PushBack(message);
            }
        }
        return 0;
    }
}