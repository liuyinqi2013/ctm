#include "netserver.h"
#include "common/com.h"
#include "common/macro.h"
#include "common/string_tools.h"

#include <sys/epoll.h>
#include <signal.h>

#define BUF_LEN          4096
#define MAX_WAIT_EVENTS  100
#define MAX_EPOLL_FDS    1024

namespace ctm
{
	void Thread_PIPE(int sign)
	{
		DEBUG("Recv sign SIGPIPE thread id : %d", pthread_self());
	}
	
	int CTcpNetServer::CNetSendThread::Run()
	{
		//signal(SIGPIPE, Thread_PIPE);
		while(1)
		{
			m_tcpNetServer->ClientSend();
		}

		return 0;
	}

	int CTcpNetServer::CNetRecvThread::Run()
	{
		//signal(SIGPIPE, Thread_PIPE);
		while(1)
		{
			m_tcpNetServer->ClientRecv();
		}

		return 0;
	}
	
	
	CTcpNetServer::CTcpNetServer(const std::string& ip, int port) :
		m_strIp(ip),
		m_iPort(port),
		m_epollFd(-1),
		m_sendThreadNum(1),
		m_recvThreadNum(1),
		m_pSendThread(NULL),
		m_pRecvThread(NULL),
		m_endFlag("\r\n"),
		m_netPackPool(128)
	{
	
	}

	CTcpNetServer::~CTcpNetServer()
	{
		ShutDown();
	}

	bool CTcpNetServer::Init()
	{
		m_epollFd = epoll_create(MAX_EPOLL_FDS);
		if (-1 == m_epollFd)
		{
			ERROR("epoll_create1 failed");
			return false;
		}

		if(!m_sockFd.IsValid())
		{
			ERROR("m_tcpSock INVALID!");
			return false;
		}

		if (!m_sockFd.SetKeepAlive(600))
		{
			ERROR("errcode = %d, errmsg = %s!", m_sockFd.GetErrCode(), m_sockFd.GetErrMsg().c_str());
			return false;
		}
		
		if(!m_sockFd.Bind(m_strIp, m_iPort))
		{
			ERROR("errcode = %d, errmsg = %s!", m_sockFd.GetErrCode(), m_sockFd.GetErrMsg().c_str());
			return false;
		}

		if(!m_sockFd.Listen(SOMAXCONN))
		{
			ERROR("errcode = %d, errmsg = %s!", m_sockFd.GetErrCode(), m_sockFd.GetErrMsg().c_str());
			return false;
		}

		struct epoll_event event = {0};
		event.events  = EPOLLIN;
		event.data.fd = m_sockFd.GetSock();
	
		int iRet = epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_sockFd.GetSock(), &event);
		if (iRet != 0)
		{
			ERROR("epoll_ctl failed");
			return false;
		}

		m_pSendThread = new CNetSendThread[m_sendThreadNum];
		if (!m_pSendThread)
		{
			ERROR("create send thread failed");
			return false;			
		}

		for (int i = 0; i < m_sendThreadNum; ++i)
		{
			m_pSendThread[i].SetName(std::string("NetSendThread_") + I2S(i));
			m_pSendThread[i].SetNetServer(this);
		}

		m_pRecvThread = new CNetRecvThread[m_recvThreadNum];
		if (!m_pRecvThread)
		{
			ERROR("create recv thread failed");
			return false;			
		}

		for (int i = 0; i < m_recvThreadNum; ++i)
		{
			m_pRecvThread[i].SetName(std::string("NetRecvThread_") + I2S(i));
			m_pRecvThread[i].SetNetServer(this);
		}
		
		return true;
	}

	void CTcpNetServer::ShutDown()
	{
		DEBUG("Tcp net server shutdown");
		
		CLockOwner owner(m_mutexLock);

		Stop();

		if (m_pRecvThread)
		{
			for (int i = 0; i < m_recvThreadNum; ++i)
			{
				m_pRecvThread[i].Stop();
			}

			delete [] m_pRecvThread;
		}

		if (m_pSendThread)
		{
			for (int i = 0; i < m_sendThreadNum; ++i)
			{
				m_pSendThread[i].Stop();
			}

			delete [] m_pSendThread;
		}
		
		std::map<SOCKET_T, ClientConnect*>::iterator it = m_mapConns.begin();
		for (; it != m_mapConns.end(); it++)
		{
			if (!it->second)
			{
				it->second->m_ConnSock.Close();
				delete it->second;
				it->second = NULL;
			}
		}

		if (-1 == m_epollFd)
		{
			close(m_epollFd);
		}

		m_sockFd.Close();
	}

	void CTcpNetServer::StartUp()
	{
		if (m_pRecvThread)
		{
			for (int i = 0; i < m_recvThreadNum; ++i)
			{
				m_pRecvThread[i].Start();
			}
		}

		if (m_pSendThread)
		{
			for (int i = 0; i < m_sendThreadNum; ++i)
			{
				m_pSendThread[i].Start();
			}
		}

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
		ClientConnect* conn = NULL;
		
		while(1)
		{
			nfds = epoll_wait(m_epollFd, events, MAX_WAIT_EVENTS, -1);
			if (nfds == -1) 
			{
				ERROR("epoll_wait failed");
				return -1;
			}
			
			for ( i = 0; i < nfds; ++i)
			{
				if (events[i].data.fd == m_sockFd.GetSock())
				{
					clientSock = m_sockFd.Accept(strClientIp, iClientPort);
					DEBUG("clientSock fd = %d", clientSock.GetSock());
					if (!clientSock.IsValid())
					{
						ERROR("errcode = %d, errmsg = %s!", m_sockFd.GetErrCode(), m_sockFd.GetErrMsg().c_str());
						if (m_sockFd.GetErrCode() == EINTR)
							continue;
						
						return -1;
					}
					
					conn = new ClientConnect(clientSock, strClientIp, iClientPort);
					
					if (!AddClientConnect(conn))
						continue;
					
					CNetPack* pNetPack = m_netPackPool.Get();
					if (pNetPack)
					{
						pNetPack->sock = conn->m_ConnSock.GetSock();
						strcpy(pNetPack->ip, conn->m_strConnIp.c_str());
						pNetPack->port = conn->m_iConnPort;
						pNetPack->ilen = strlen("Login");
						strncpy(pNetPack->ibuf, "Login", BUF_MAX_SIZE);
						pNetPack->ibuf[pNetPack->ilen] = 0x0;
						pNetPack->olen = 0;
						pNetPack->obuf[0] = 0x0;
						m_recvQueue.Push(pNetPack);
						DEBUG("Net Put Login");
					}
					
				}
				else if (events[i].events & EPOLLRDHUP)
				{
					DEBUG("Socket %d peer shutdown", events[i].data.fd);
					/*
					if (ReadClientConnect(m_mapConns[events[i].data.fd]) != 0)
						DelClientConnect(events[i].data.fd);
					*/
					m_mapConns[events[i].data.fd]->m_iStatus = 1;
					m_readableConnQueue.Push(m_mapConns[events[i].data.fd]);
				}
				else if (events[i].events & EPOLLERR)
				{
					DEBUG("Socket %d is happend error", events[i].data.fd);
					DelClientConnect(events[i].data.fd);
				}	
				else if (events[i].events & EPOLLIN)
				{
					//DEBUG("Socket %d is Readable", events[i].data.fd);
					//CReadThread::PutMsg(m_mapConns[events[i].data.fd]);
					//ReadOnePacket(m_mapConns[events[i].data.fd]);
					/*
					if (ReadClientConnect(m_mapConns[events[i].data.fd]) == -1)
						DelClientConnect(events[i].data.fd);
					*/
					m_mapConns[events[i].data.fd]->m_iStatus = 0;
					m_readableConnQueue.Push(m_mapConns[events[i].data.fd]);
					
				}

			}
			
		}
		
		return 0;
	}

	ClientConnect* CTcpNetServer::GetClientConnect(SOCKET_T sock)
	{
		ClientConnect* p = NULL;
		CLockOwner owner(m_mutexLock);
		std::map<SOCKET_T, ClientConnect*>::iterator it = m_mapConns.find(sock);
		if (it != m_mapConns.end())
			p = it->second;

		return p;
	}
	
	bool CTcpNetServer::AddClientConnect(ClientConnect* conn)
	{
		DEBUG("BEGIN");
		if (!conn) return false;

			
		CLockOwner owner(m_mutexLock);
			
		conn->m_ConnSock.SetNonBlock();
		
		struct epoll_event event = {0};
		event.events  = EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLET;
		event.data.fd = conn->m_ConnSock.GetSock();
		DEBUG("Add new connect [%s]", conn->ToString().c_str());
		int iRet = epoll_ctl(m_epollFd, EPOLL_CTL_ADD, conn->m_ConnSock.GetSock(), &event);
		if (iRet != 0)
		{
			int errCode = GetSockErrCode();
			std::string errMsg  = GetSockErrMsg(errCode);	
			ERROR("epoll_ctl add failed fd : %d errno = %d, errmsg = %s", conn->m_ConnSock.GetSock(), errCode, errMsg.c_str());
			
			conn->m_ConnSock.Close();
			delete conn;
			conn = NULL;
			return false;

		}
		m_mapConns[conn->m_ConnSock.GetSock()] = conn;
		DEBUG("END");

		return true;
	} 

	bool CTcpNetServer::DelClientConnect(SOCKET_T sock)
	{
		DEBUG("BEGIN");
		
		CLockOwner owner(m_mutexLock);

		/*
		CNetPack* pNetPack = GetContext(sock); 
		if (pNetPack)
		{
			DelContext(pNetPack->sock);
			m_netPackPool.Recycle(pNetPack);
		}
		*/

		m_Context.Remove(sock);
					
		std::map<SOCKET_T, ClientConnect*>::iterator it = m_mapConns.find(sock);
		if (it != m_mapConns.end())
		{
			DEBUG("find");
			ClientConnect* conn = it->second;
			if (!conn) return false;


			int iRet = epoll_ctl(m_epollFd, EPOLL_CTL_DEL, conn->m_ConnSock.GetSock(), NULL);
			DEBUG("Delete connect [%s]", conn->ToString().c_str());
			if (iRet != 0)
			{
				int errCode = GetSockErrCode();
				std::string errMsg  = GetSockErrMsg(errCode);
				
				ERROR("epoll_ctl del failed fd : %d errno = %d, errmsg = %s", conn->m_ConnSock.GetSock(), errCode, errMsg.c_str());
			}

			CSystemNetMsg systemNetMsg(conn->m_iConnPort, conn->m_strConnIp, conn->m_ConnSock.GetSock(), 2);
			CNetPack* pNetPack = m_netPackPool.Get();
			if (pNetPack)
			{
					pNetPack->sock = conn->m_ConnSock.GetSock();
					strcpy(pNetPack->ip, conn->m_strConnIp.c_str());
					pNetPack->port = conn->m_iConnPort;
					pNetPack->ilen = systemNetMsg.ToString().size();
					strncpy(pNetPack->ibuf, systemNetMsg.ToString().c_str(), BUF_MAX_SIZE);
					pNetPack->ibuf[pNetPack->ilen] = 0x0;
					pNetPack->olen = 0;
					pNetPack->obuf[0] = 0x0;
					m_recvQueue.Push(pNetPack);
					DEBUG("Net off line");
			}
			
			conn->m_ConnSock.Close();
			delete conn;
			conn = NULL;
			m_mapConns.erase(it);
		}
		
		DEBUG("END");
		return true;
	}

	bool CTcpNetServer::DelClientConnect(ClientConnect* conn)
	{
		if (!conn) return false;

		return DelClientConnect(conn->m_ConnSock.GetSock());
	}

	void CTcpNetServer::ClientRecv()
	{
		ClientConnect* pConn = m_readableConnQueue.GetAndPop();
		if (pConn)
		{
			int ret = ReadClientConnect(pConn);
			if (ret == -1 || (ret == 1 && pConn->m_iStatus == 1))
			{
				DelClientConnect(pConn);
			}
		}
	}
	
	void CTcpNetServer::ClientSend()
	{
		CNetPack* pNetPack   = m_sendQueue.GetAndPop();
		ClientConnect* pConn = GetClientConnect(pNetPack->sock);
		if (pConn)
		{
			std::string buf = m_Context.Pack(std::string(pNetPack->obuf, pNetPack->olen));
			int len = pConn->m_ConnSock.Send(buf.data(), buf.size());
			if (len <= 0)
			{
				ERROR("errcode = %d, errmsg = %s!", pConn->m_ConnSock.GetErrCode(), pConn->m_ConnSock.GetErrMsg().c_str());
			}
		}

		m_netPackPool.Recycle(pNetPack);
	}

	int CTcpNetServer::ReadClientConnect(ClientConnect* conn)
	{
		std::string strOut;
		char buf[BUF_LEN + 32] = {0};
		int buflen = BUF_LEN;
		int len  = 0;
		int errCode = 0;
		std::string errMsg;
		std::vector<std::string> vecOutput;

		int ret = 0;
		
		while (1)
		{
			len = conn->m_ConnSock.Recv(buf, buflen);
			if (len <= 0)
			{
				errCode = GetSockErrCode();
				errMsg  = GetSockErrMsg(errCode);
				if (errCode == EINTR)
				{
					continue;
				}
				
				if ((errCode == EWOULDBLOCK || errCode == EAGAIN))
				{
					ret = 1;
				}
				else
				{
					DEBUG("errcode = %d, errmsg = %s!", errCode, errMsg.c_str());
					ret = -1;
				}
				
				break;
			}
			else
			{
				buf[len] = '\0';

				m_Context.GetCompletePack(conn->m_ConnSock.GetSock(), std::string(buf, len), vecOutput);
				
				for (size_t i = 0; i < vecOutput.size(); ++i)
				{
					CNetPack* pNetPack = m_netPackPool.Get();
						
					pNetPack->sock = conn->m_ConnSock.GetSock();
					strcpy(pNetPack->ip, conn->m_strConnIp.c_str());
					pNetPack->port = conn->m_iConnPort;
					pNetPack->ilen = vecOutput[i].size();
					memcpy(pNetPack->ibuf, vecOutput[i].data(), pNetPack->ilen);
					pNetPack->ibuf[pNetPack->ilen] = 0x0;
					pNetPack->olen = 0;
					pNetPack->obuf[0] = 0x0;
					m_recvQueue.Push(pNetPack);
				}
			}
		}
		
		return ret;
	}

	int CTcpNetServer::ReadOnePacket(ClientConnect* conn)
	{
		DEBUG("BEGIN");
		int dataLen = 0;

		if (Readn(conn, (char*)&dataLen, sizeof(dataLen)) < 0)
		{
			DEBUG("ip = %s, port = %d read 4 bit data failed", conn->m_strConnIp.c_str(), conn->m_iConnPort);
			return -1;
		}
		
		/*
		int len = conn->m_ConnSock.Recv((char*)&dataLen, sizeof(dataLen));
		if (len < sizeof(dataLen)) 
		{
			DEBUG("ip = %s, port = %d, len = %d read 4 bit data failed", conn->m_strConnIp.c_str(), conn->m_iConnPort, len);
			DelClientConnect(conn);
			return -1;
		}
		*/

		dataLen = ntohl(dataLen);
		DEBUG("Content dataLen = %d\n", dataLen);

		char *buf = new char[dataLen + 1];
		if (!buf)
		{
			DEBUG("malloc mem failed");
			return -1;
		}

		memset(buf, 0, dataLen + 1);

		std::string strContent;
		std::string errMsg;
		
		if (Readn(conn, buf, dataLen) < 0)
		{
			DEBUG("Readn failed");
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
				//DEBUG("errcode = %d, errmsg = %s!", errCode, errMsg.c_str());
				if (errCode == EINTR) 
				{
					continue;
				}
				else if (errCode == EWOULDBLOCK || errCode == EAGAIN)
				{
					//DEBUG("ip = %s, port = %d, len = %d need wait data", conn->m_strConnIp.c_str(), conn->m_iConnPort, len);
					usleep(1000);
					continue;
				}
				else
				{
					DelClientConnect(conn);
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
		

		//DEBUG("ip = %s, port = %d, len = %d, recv = %s", conn->m_strConnIp.c_str(), conn->m_iConnPort, dataLen, buf);
		
		CNetPack* pNetPack = m_netPackPool.Get();
		if (pNetPack)
		{
			pNetPack->sock = conn->m_ConnSock.GetSock();
			strcpy(pNetPack->ip, conn->m_strConnIp.c_str());
			pNetPack->port = conn->m_iConnPort;
			pNetPack->ilen = dataLen;
			memcpy(pNetPack->ibuf, buf, BUF_MAX_SIZE);
			pNetPack->ibuf[pNetPack->ilen] = 0x0;
			pNetPack->olen = 0;
			pNetPack->obuf[0] = 0x0;
			m_recvQueue.Push(pNetPack);
		}

		/*
		CNetMsg* pNetMsg = new CNetMsg(conn->m_iConnPort, conn->m_strConnIp, conn->m_ConnSock, buf);
		if (!m_RecvQueue.Put(pNetMsg))
		{
			delete pNetMsg;
			pNetMsg = NULL;
		}
		*/

		delete[] buf;
		
		DEBUG("END");
		return 0;
	}


	int CTcpNetServer::Readn(ClientConnect* conn, char* buf, int len)
	{
		DEBUG("BEGIN");
		int offset  = 0;
		int errCode = 0;
		int length  = 0;
		std::string errMsg;
		while (offset < len)
		{
			length = conn->m_ConnSock.Recv(buf + offset, len - offset);
			DEBUG("len = %d, length = %d, offset = %d", len, length, offset);
			if (length <= 0)
			{
				errCode = GetSockErrCode();
				errMsg  = GetSockErrMsg(errCode);
				DEBUG("errcode = %d, errmsg = %s!", errCode, errMsg.c_str());
				if (errCode == EINTR)
				{
					continue;
				}
				else if (errCode == EWOULDBLOCK || errCode == EAGAIN)
				{
					DEBUG("ip = %s, port = %d, len = %d need wait data", conn->m_strConnIp.c_str(), conn->m_iConnPort, len);
					usleep(1000);
					continue;
				}
				else
				{
					DelClientConnect(conn);
					return -1;
				}
			}
			else
			{
				offset += length;
			}
		}
		
		DEBUG("END");
		
		return len;
	}
}