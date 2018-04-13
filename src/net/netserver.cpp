#include "netserver.h"
#include "common/macro.h"

#include <sys/epoll.h>

#define BUF_LEN          1024
#define MAX_WAIT_EVENTS  100
#define MAX_EPOLL_FDS    1024

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
		m_epollFd = epoll_create(MAX_EPOLL_FDS);
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

		//设置空闲时间为30分钟
		if (!m_sockFd.SetKeepAlive(18000))
		{
			ERROR_LOG("errcode = %d, errmsg = %s!", m_sockFd.GetErrCode(), m_sockFd.GetErrMsg().c_str());
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

	void CTcpNetServer::StartUp()
	{
		Start();
	}
	
	int CTcpNetServer::Run()
	{
		int i = 0;
		int nfds = 0;
		struct epoll_event events[MAX_WAIT_EVENTS];
		std::string strClientIp;
		int iClientPort = 0;
		CSocket clientSock;
		ConnInfo* conn = NULL;
		
		while(1)
		{
			nfds = epoll_wait(m_epollFd, events, MAX_WAIT_EVENTS, -1);
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
						if (m_sockFd.GetErrCode() == EINTR)
							continue;
						
						return -1;
					}

					DEBUG_LOG("Connect ip = %s, port = %d", strClientIp.c_str(), iClientPort);
					conn = new ConnInfo(clientSock, strClientIp, iClientPort);
					AddClientConn(conn);
					
					CNetMsg* pNetMsg = new CNetMsg(conn->m_iConnPort, conn->m_strConnIp, conn->m_ConnSock, "Login");
					if (!m_RecvQueue.Put(pNetMsg))
					{
						delete pNetMsg;
						pNetMsg = NULL;
					}
				}
				else if (events[i].events & EPOLLIN)
				{
					DEBUG_LOG("Socket %d is Readable", events[i].data.fd);
					//CReadThread::PutMsg(m_mapConns[events[i].data.fd]);
					//ReadClientConn(m_mapConns[events[i].data.fd]);
					ReadOnePacket(m_mapConns[events[i].data.fd]);
				}
				else if (events[i].events & EPOLLERR)
				{
					DEBUG_LOG("Socket %d is happend error", events[i].data.fd);
					DelClientConn(events[i].data.fd);
				}
				else if (events[i].events & EPOLLRDHUP)
				{
					DEBUG_LOG("Socket %d peer shutdown", events[i].data.fd);
					DelClientConn(events[i].data.fd);
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
			
		/*conn->m_ConnSock.SetNonBlock();*/
		m_mapConns[conn->m_ConnSock.GetSock()] = conn;
		struct epoll_event event = {0};
		event.events  = EPOLLIN | EPOLLERR | EPOLLRDHUP /*| EPOLLET*/;
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

	int CTcpNetServer::ReadClientConn(ConnInfo* conn)
	{
		std::string strOut;
		char buf[BUF_LEN] = {0};
		int buflen = BUF_LEN;
		int offset = 0;
		int   len  = 0;
		int errCode = 0;
		std::string errMsg;
		
		while (1)
		{
			len = conn->m_ConnSock.Recv(buf, buflen);
			if (len <= 0)
			{
				errCode = GetSockErrCode();
				errMsg  = GetSockErrMsg(errCode);
				DEBUG_LOG("errcode = %d, errmsg = %s!", errCode, errMsg.c_str());
				if ((errCode == EWOULDBLOCK || errCode == EAGAIN) && offset)//需要等待资源 
				{
					DEBUG_LOG("ip = %s, port = %d, len = %d, recv = %s", conn->m_strConnIp.c_str(), conn->m_iConnPort, strOut.size(), strOut.c_str());
					CNetMsg* pNetMsg = new CNetMsg(conn->m_iConnPort, conn->m_strConnIp, conn->m_ConnSock, strOut);
					if (!m_RecvQueue.Put(pNetMsg))
					{
						delete pNetMsg;
						pNetMsg = NULL;
					}
				
				}
				else
				{
					DelClientConn(conn);
				}
				return 1;
				
			}
			else
			{
				buf[len] = '\0';
				strOut.append(buf, len);
				offset += len;
			}
		}
		
		return 0;
	}

	int CTcpNetServer::ReadOnePacket(ConnInfo* conn)
	{
		DEBUG_LOG("BEGIN");
		//先读取一个长度
		int dataLen = 0;

		
		if (Readn(conn, (char*)&dataLen, sizeof(dataLen)) < 0)
		{
			DEBUG_LOG("ip = %s, port = %d read 4 bit data failed", conn->m_strConnIp.c_str(), conn->m_iConnPort);
			return -1;
		}
		
		
		/*
		int len = conn->m_ConnSock.Recv((char*)&dataLen, sizeof(dataLen));
		if (len < sizeof(dataLen)) //4个字节都读不了
		{
			DEBUG_LOG("ip = %s, port = %d, len = %d read 4 bit data failed", conn->m_strConnIp.c_str(), conn->m_iConnPort, len);
			DelClientConn(conn);
			return -1;
		}
		*/

		dataLen = ntohl(dataLen);
		DEBUG_LOG("Content dataLen = %d\n", dataLen);

		//再根据长度读取内容
		char *buf = new char[dataLen + 1];
		if (!buf)
		{
			DEBUG_LOG("malloc mem failed");
			return -1;
		}

		memset(buf, 0, dataLen + 1);

		
		std::string strContent;
		int offset  = 0;
		int errCode = 0;
		std::string errMsg;
		

		
		if (Readn(conn, buf, dataLen) < 0)
		{
			DEBUG_LOG("Readn failed");
			delete[] buf;
			return -1;
		}
		
		
		/*
		while (offset < dataLen)
		{
			len = conn->m_ConnSock.Recv(buf + offset, dataLen - offset);
			if (len <= 0)
			{
				errCode = GetSockErrCode();
				errMsg  = GetSockErrMsg(errCode);
				//DEBUG_LOG("errcode = %d, errmsg = %s!", errCode, errMsg.c_str());
				if (errCode == EINTR) //被一个捕获的信号中断
				{
					continue;
				}
				else if (errCode == EWOULDBLOCK || errCode == EAGAIN)//需要等待资源 
				{
					//DEBUG_LOG("ip = %s, port = %d, len = %d need wait data", conn->m_strConnIp.c_str(), conn->m_iConnPort, len);
					usleep(1000);
					continue;
				}
				else
				{
					DelClientConn(conn);
					delete buf;
					return -1;
				}
			}
			else
			{
				offset += len;
			}
		}
		*/
		

		//DEBUG_LOG("ip = %s, port = %d, len = %d, recv = %s", conn->m_strConnIp.c_str(), conn->m_iConnPort, dataLen, buf);
		CNetMsg* pNetMsg = new CNetMsg(conn->m_iConnPort, conn->m_strConnIp, conn->m_ConnSock, buf);
		if (!m_RecvQueue.Put(pNetMsg))
		{
			delete pNetMsg;
			pNetMsg = NULL;
		}

		delete[] buf;
		DEBUG_LOG("END");
		return 0;
	}

	int CTcpNetServer::Readn(ConnInfo* conn, char* buf, int len)
	{
		DEBUG_LOG("BEGIN");
		int offset  = 0;
		int errCode = 0;
		int length  = 0;
		std::string errMsg;
		while (offset < len)
		{
			length = conn->m_ConnSock.Recv(buf + offset, len - offset);
			DEBUG_LOG("************** len = %d, length = %d, offset = %d", len, length, offset);
			if (length <= 0)
			{
				errCode = GetSockErrCode();
				errMsg  = GetSockErrMsg(errCode);
				DEBUG_LOG("errcode = %d, errmsg = %s!", errCode, errMsg.c_str());
				if (errCode == EINTR) //被一个捕获的信号中断
				{
					DEBUG_LOG("被一个捕获的信号中断");
					continue;
				}
				else if (errCode == EWOULDBLOCK || errCode == EAGAIN)//需要等待资源 
				{
					DEBUG_LOG("ip = %s, port = %d, len = %d need wait data", conn->m_strConnIp.c_str(), conn->m_iConnPort, len);
					usleep(1000);
					continue;
				}
				else
				{
					DelClientConn(conn);
					return -1;
				}
			}
			else
			{
				offset += length;
			}
		}
		
		DEBUG_LOG("END");
		
		return len;
	}

}