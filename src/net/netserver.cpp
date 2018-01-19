#include "netserver.h"
#include "common/macro.h"

#include <sys/epoll.h>

#define MAX_EVENTS 100

namespace ctm
{
	CTcpNetServer::CTcpNetServer(const std::string& ip, int port) :
		CThread("TcpNetServerThread"),
		m_strIp(ip),
		m_iPort(port),
		m_epollFd(-1)
		
	{
	}

	CTcpNetServer::~CTcpNetServer()
	{
		CLockOwner owner(m_mutexLock);
		if (GetStatus() == T_RUN) Stop();

		std::map<SOCKET_T, ConnInfo*>::iterator it = m_mapConns.begin();
		for (; it != m_mapConns.end(); it++)
		{
			if (!it->second)
			{
				delete it->second;
				it->second = NULL;
			}
		}

		if (-1 == m_epollFd)
		{
			close(m_epollFd);
		}
		
	}

	bool CTcpNetServer::Init()
	{
		m_epollFd = epoll_create(1024);
		if (-1 == m_epollFd)
		{
			ERROR_LOG("epoll_create1 failed");
			return false;
		}

		if(!m_sockFd.IsValid())
		{
			ERROR_LOG("m_tcpSock INVALID!");
			return false;
		}

		if(!m_sockFd.Bind(m_strIp, m_iPort))
		{
			ERROR_LOG("errcode = %d, errmsg = %s!", m_sockFd.GetErrCode(), m_sockFd.GetErrMsg().c_str());
			return false;
		}

		if(!m_sockFd.Listen(SOMAXCONN))
		{
			ERROR_LOG("errcode = %d, errmsg = %s!", m_sockFd.GetErrCode(), m_sockFd.GetErrMsg().c_str());
			return false;
		}

		struct epoll_event event = {0};
		event.events  = EPOLLIN;
		event.data.fd = m_sockFd.GetSock();
	
		int iRet = epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_sockFd.GetSock(), &event);
		if (iRet != 0)
		{
			ERROR_LOG("epoll_ctl failed");
			return false;
		}
		
		return true;
	}
	
	int CTcpNetServer::Run()
	{
		int i = 0;
		int nfds = 0;
		struct epoll_event events[MAX_EVENTS];
		std::string strClientIp;
		int iClientPort = 0;
		int len =  0;
		CSocket clientSock;
		ConnInfo* conn = NULL;
		
		while(1)
		{
			nfds = epoll_wait(m_epollFd, events, MAX_EVENTS, -1);
			if (nfds == -1) 
			{
				ERROR_LOG("epoll_wait failed");
				return -1;
			}

			DEBUG_LOG("reader nfds = %d", nfds);
			for ( i = 0; i < nfds; ++i)
			{
				if (events[i].data.fd == m_sockFd.GetSock())
				{
					clientSock = m_sockFd.Accept(strClientIp, iClientPort);
					DEBUG_LOG("clientSock fd = %d", clientSock.GetSock());
					if (!clientSock.IsValid())
					{
						ERROR_LOG("errcode = %d, errmsg = %s!", m_sockFd.GetErrCode(), m_sockFd.GetErrMsg().c_str());
						return -1;
					}

					DEBUG_LOG("Connect ip = %s, port = %d", strClientIp.c_str(), iClientPort);
					conn = new ConnInfo(clientSock, strClientIp, iClientPort);
					AddClientConn(conn);
				}
				else if (events[i].events & EPOLLIN)
				{
					DEBUG_LOG("oooo");
					HandleReadConn(m_mapConns[events[i].data.fd]);

				}
			}
			
		}
		
		return 0;
	}

	void CTcpNetServer::AddClientConn(ConnInfo* conn)
	{
		DEBUG_LOG("BEGIN");
		if (!conn) return;
			
		CLockOwner owner(m_mutexLock);
			
		conn->m_ConnSock.SetNonBlock();
		m_mapConns[conn->m_ConnSock.GetSock()] = conn;
		struct epoll_event event = {0};
		event.events  = EPOLLIN | EPOLLET;
		event.data.fd = conn->m_ConnSock.GetSock();
	
		int iRet = epoll_ctl(m_epollFd, EPOLL_CTL_ADD, conn->m_ConnSock.GetSock(), &event);
		if (iRet != 0)
		{
			ERROR_LOG("epoll_ctl failed");
			delete conn;
			conn = NULL;
		}
		DEBUG_LOG("END");
		
	} 

	void CTcpNetServer::DelClientConn(SOCKET_T sock)
	{
		DEBUG_LOG("BEGIN");
		CLockOwner owner(m_mutexLock);
		std::map<SOCKET_T, ConnInfo*>::iterator it = m_mapConns.find(sock);
		if (it != m_mapConns.end())
		{
			DEBUG_LOG("find");
			ConnInfo* conn = it->second;
			if (!conn) return;
			int iRet = epoll_ctl(m_epollFd, EPOLL_CTL_DEL, conn->m_ConnSock.GetSock(), NULL);
			if (iRet != 0)
			{
				ERROR_LOG("epoll_ctl failed");
			}
			
			delete conn;
			conn = NULL;
			m_mapConns.erase(it);
		}
		DEBUG_LOG("END");
	}

	void CTcpNetServer::DelClientConn(ConnInfo* conn)
	{
		if (!conn) return;
		DelClientConn(conn->m_ConnSock.GetSock());
	}

	int CTcpNetServer::HandleReadConn(ConnInfo* conn)
	{
		char buf[1024] = {0};
		int buflen = 1024;
		int offset = 0;
		char* send = "hello client";
		int   len  = 0;
		while (1)
		{
			len = conn->m_ConnSock.Recv(buf + offset, buflen - offset  - 1);
			if (len < 0)
			{
				DEBUG_LOG("errcode = %d, errmsg = %s!", conn->m_ConnSock.GetErrCode(), conn->m_ConnSock.GetErrMsg().c_str());
				int errCode = conn->m_ConnSock.GetErrCode();
				if (errCode == EWOULDBLOCK || errCode == EAGAIN) 
				{
					buf[offset] = '\0';
					DEBUG_LOG("ip = %s, port = %d, len = %d, recv = %s", conn->m_strConnIp.c_str(), conn->m_iConnPort, offset, buf);
					len = conn->m_ConnSock.Send(send, strlen(send));
					if (len == -1) 
					{
						ERROR_LOG("errcode = %d, errmsg = %s!", conn->m_ConnSock.GetErrCode(), conn->m_ConnSock.GetErrMsg().c_str());
					}
					DEBUG_LOG("ip = %s, port = %d, len = %d, send = %s", conn->m_strConnIp.c_str(), conn->m_iConnPort, offset, send);
				
				}
				else
				{
				
				}
				
				return 1;
			}
			else if (len == 0)
			{
				ERROR_LOG("errcode = %d, errmsg = %s!", conn->m_ConnSock.GetErrCode(), conn->m_ConnSock.GetErrMsg().c_str());
				int errCode = conn->m_ConnSock.GetErrCode();
				if ((errCode == EWOULDBLOCK || errCode == EAGAIN) && offset) 
				{
					buf[offset] = '\0';
					DEBUG_LOG("ip = %s, port = %d, len = %d, recv = %s", conn->m_strConnIp.c_str(), conn->m_iConnPort, offset, buf);
					len = conn->m_ConnSock.Send(send, strlen(send));
					if (len == -1) 
					{
						ERROR_LOG("errcode = %d, errmsg = %s!", conn->m_ConnSock.GetErrCode(), conn->m_ConnSock.GetErrMsg().c_str());
					}
					DEBUG_LOG("ip = %s, port = %d, len = %d, send = %s", conn->m_strConnIp.c_str(), conn->m_iConnPort, offset, send);
				
				}
				else
				{
					DelClientConn(conn);
				}
				
				return 2;
			}
			else
			{
				offset += len;
			}
		}
		
		return 0;
	}
}