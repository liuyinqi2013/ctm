#include "netserver.h"
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
		DEBUG_LOG("Recv sign SIGPIPE thread id : %d", pthread_self());
	}
	
	int CSendThread::Run()
	{
		signal(SIGPIPE, Thread_PIPE);
		while(1)
		{
			//DEBUG_LOG("get send net pack");
			CNetPack* pNetPack = m_tcpNetServer->m_netPackCache.SendPack();
			CliConn* pConn = m_tcpNetServer->GetCliConn(pNetPack->sock);
			if (pConn)
			{
				//DEBUG_LOG("Send : %s", pNetPack->obuf);
				int len = pConn->m_ConnSock.Send(pNetPack->obuf, pNetPack->olen);
				if (len <= 0)
				{
					ERROR_LOG("errcode = %d, errmsg = %s!", pConn->m_ConnSock.GetErrCode(), pConn->m_ConnSock.GetErrMsg().c_str());
					//m_tcpNetServer->DelCliConn(pConn);
				}
			}

			m_tcpNetServer->m_netPackCache.PutFreeQueue(pNetPack);
		}

		return 0;
	}
	
	CTcpNetServer::CTcpNetServer(const std::string& ip, int port) :
		CThread("TcpNetServerThread"),
		m_strIp(ip),
		m_iPort(port),
		m_epollFd(-1),
		m_endFlag("\r\n")
	{
		m_sendThread.SetNetServer(this);
	}

	CTcpNetServer::~CTcpNetServer()
	{
		CLockOwner owner(m_mutexLock);
		if (GetStatus() == T_RUN) Stop();

		std::map<SOCKET_T, CliConn*>::iterator it = m_mapConns.begin();
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
		if (!m_sockFd.SetKeepAlive(600))
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
		m_sendThread.Start();
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
		CliConn* conn = NULL;
		
		while(1)
		{
			nfds = epoll_wait(m_epollFd, events, MAX_WAIT_EVENTS, -1);
			if (nfds == -1) 
			{
				ERROR_LOG("epoll_wait failed");
				return -1;
			}

			//DEBUG_LOG("reader nfds = %d", nfds);
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
					conn = new CliConn(clientSock, strClientIp, iClientPort);
					
					if (!AddCliConn(conn))
						continue;

					/*
					CNetMsg* pNetMsg = new CNetMsg(conn->m_iConnPort, conn->m_strConnIp, conn->m_ConnSock, "Login");
					if (!m_RecvQueue.Put(pNetMsg))
					{
						delete pNetMsg;
						pNetMsg = NULL;
					}
					*/
					
					CNetPack* pNetPack = m_netPackCache.FreePack();
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
						m_netPackCache.PutRecvQueue(pNetPack);
						DEBUG_LOG("Net Put Login");
					}
					
				}
				else if (events[i].events & EPOLLERR)
				{
					DEBUG_LOG("Socket %d is happend error", events[i].data.fd);
					DelCliConn(events[i].data.fd);
				}
				else if (events[i].events & EPOLLRDHUP)
				{
					DEBUG_LOG("Socket %d peer shutdown", events[i].data.fd);
					DelCliConn(events[i].data.fd);
				}
				else if (events[i].events & EPOLLIN)
				{
					//DEBUG_LOG("Socket %d is Readable", events[i].data.fd);
					//CReadThread::PutMsg(m_mapConns[events[i].data.fd]);
					ReadCliConn(m_mapConns[events[i].data.fd]);
					//ReadOnePacket(m_mapConns[events[i].data.fd]);
				}
			}
			
		}
		
		return 0;
	}

	CliConn* CTcpNetServer::GetCliConn(SOCKET_T sock)
	{
		//DEBUG_LOG("BEGIN");
		CliConn* p = NULL;
		CLockOwner owner(m_mutexLock);
		std::map<SOCKET_T, CliConn*>::iterator it = m_mapConns.find(sock);
		if (it != m_mapConns.end())
			p = it->second;
		//DEBUG_LOG("END");
		return p;
	}
	
	bool CTcpNetServer::AddCliConn(CliConn* conn)
	{
		DEBUG_LOG("BEGIN");
		if (!conn) return false;
			
		CLockOwner owner(m_mutexLock);
			
		conn->m_ConnSock.SetNonBlock();
		
		struct epoll_event event = {0};
		event.events  = EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLET;
		event.data.fd = conn->m_ConnSock.GetSock();
	
		int iRet = epoll_ctl(m_epollFd, EPOLL_CTL_ADD, conn->m_ConnSock.GetSock(), &event);
		if (iRet != 0)
		{
			int errCode = GetSockErrCode();
			std::string errMsg  = GetSockErrMsg(errCode);	
			ERROR_LOG("epoll_ctl add failed fd : %d errno = %d, errmsg = %s", conn->m_ConnSock.GetSock(), errCode, errMsg.c_str());
			
			conn->m_ConnSock.Close();
			delete conn;
			conn = NULL;
			return false;
		}
		m_mapConns[conn->m_ConnSock.GetSock()] = conn;
		DEBUG_LOG("END");

		return true;
	} 

	bool CTcpNetServer::DelCliConn(SOCKET_T sock)
	{
		DEBUG_LOG("BEGIN");
		
		CLockOwner owner(m_mutexLock);

		//删除上下文
		CNetPack* pNetPack = m_netPackCache.GetContext(sock); 
		if (pNetPack)
		{
			m_netPackCache.DelContext(pNetPack->sock);
			m_netPackCache.PutFreeQueue(pNetPack);
		}
					
		std::map<SOCKET_T, CliConn*>::iterator it = m_mapConns.find(sock);
		if (it != m_mapConns.end())
		{
			DEBUG_LOG("find");
			CliConn* conn = it->second;
			if (!conn) return false;

			DEBUG_LOG("epoll_ctl del fd : %d", conn->m_ConnSock.GetSock());
			int iRet = epoll_ctl(m_epollFd, EPOLL_CTL_DEL, conn->m_ConnSock.GetSock(), NULL);
			if (iRet != 0)
			{
				int errCode = GetSockErrCode();
				std::string errMsg  = GetSockErrMsg(errCode);
				
				ERROR_LOG("epoll_ctl del failed fd : %d errno = %d, errmsg = %s", conn->m_ConnSock.GetSock(), errCode, errMsg.c_str());
			}
			conn->m_ConnSock.Close();
			delete conn;
			conn = NULL;
			m_mapConns.erase(it);
		}
		
		DEBUG_LOG("END");
		return true;
	}

	bool CTcpNetServer::DelCliConn(CliConn* conn)
	{
		if (!conn) return false;
		return DelCliConn(conn->m_ConnSock.GetSock());
	}

	int CTcpNetServer::ReadCliConn(CliConn* conn)
	{
		std::string strOut;
		char buf[BUF_LEN + 32] = {0};
		int buflen = BUF_LEN;
		int offset = 0;
		int   len  = 0;
		int errCode = 0;
		std::string errMsg;
		std::vector<std::string> vecOutput;
		
		while (1)
		{
			len = conn->m_ConnSock.Recv(buf, buflen);
			if (len <= 0)
			{
				errCode = GetSockErrCode();
				errMsg  = GetSockErrMsg(errCode);
				
				if (errCode == EINTR) //被一个捕获的信号中断
				{
					continue;
				}
				
				if ((errCode == EWOULDBLOCK || errCode == EAGAIN)) //需要等待资源 
				{
					//DEBUG_LOG("need wait data");
				}
				else
				{
					DEBUG_LOG("errcode = %d, errmsg = %s!", errCode, errMsg.c_str());	
					DelCliConn(conn);
				}
				
				break;
			}
			else
			{
				buf[len] = '\0';
				//DEBUG_LOG("ip = %s, port = %d, len = %d, recv = %s", conn->m_strConnIp.c_str(), conn->m_iConnPort, len, buf);
				CNetPack* pNetPack = m_netPackCache.GetContext(conn->m_ConnSock.GetSock()); 
				if (!pNetPack) //不存在上下文
				{
					//DEBUG_LOG("-------------------------------------------");
					CutString(std::string(buf, len), vecOutput, m_endFlag, false);
					int i = 0;
					for (i = 0; i < vecOutput.size() - 1; ++i)
					{
						//DEBUG_LOG("vecOutput[%d]  len = %d, content = %s", i, vecOutput[i].size(), vecOutput[i].c_str());
						pNetPack = m_netPackCache.FreePack();
						
						pNetPack->sock = conn->m_ConnSock.GetSock();
						strcpy(pNetPack->ip, conn->m_strConnIp.c_str());
						pNetPack->port = conn->m_iConnPort;
						pNetPack->ilen = vecOutput[i].size();
						strncpy(pNetPack->ibuf, vecOutput[i].data(), pNetPack->ilen);
						pNetPack->ibuf[pNetPack->ilen] = 0x0;
						pNetPack->olen = 0;
						pNetPack->obuf[0] = 0x0;
						m_netPackCache.PutRecvQueue(pNetPack);
					}
					
					pNetPack = m_netPackCache.FreePack();
					pNetPack->sock = conn->m_ConnSock.GetSock();
					strcpy(pNetPack->ip, conn->m_strConnIp.c_str());
					pNetPack->port = conn->m_iConnPort;
					pNetPack->ilen = vecOutput[i].size();
					strncpy(pNetPack->ibuf, vecOutput[i].data(),  pNetPack->ilen);
					pNetPack->ibuf[pNetPack->ilen] = 0x0;
					pNetPack->olen = 0;
					pNetPack->obuf[0] = 0x0;
					
					if (EndsWith(buf, m_endFlag))
					{
						//DEBUG_LOG("11111111111111111111111111111111111111111");
						m_netPackCache.PutRecvQueue(pNetPack); //一个完整的包
					}
					else
					{
						//DEBUG_LOG("22222222222222222222222222222222222222222222");
						m_netPackCache.AddContext(pNetPack->sock, pNetPack); //不完整的包，放入上下文
					}
					
				}
				else // 存在上下文
				{
					//DEBUG_LOG("*************************************************************");
					std::string temp =  std::string(pNetPack->ibuf, pNetPack->ilen) + std::string(buf, len);
					//DEBUG_LOG("temp = %s", temp.c_str());
					CutString(temp, vecOutput, m_endFlag, false);
					
					pNetPack->sock = conn->m_ConnSock.GetSock();
					strcpy(pNetPack->ip, conn->m_strConnIp.c_str());
					pNetPack->port = conn->m_iConnPort;
					pNetPack->ilen = vecOutput[0].size();
					strncpy(pNetPack->ibuf, vecOutput[0].data(), pNetPack->ilen);
					pNetPack->ibuf[pNetPack->ilen] = 0x0;
					pNetPack->olen = 0;
					pNetPack->obuf[0] = 0x0;
					
					if (vecOutput.size() > 1)
					{
						m_netPackCache.DelContext(pNetPack->sock);
						m_netPackCache.PutRecvQueue(pNetPack);		
						
						int i = 1;
						for (i = 1; i < vecOutput.size() - 1; ++i)
						{
							pNetPack = m_netPackCache.FreePack();
							
							pNetPack->sock = conn->m_ConnSock.GetSock();
							strcpy(pNetPack->ip, conn->m_strConnIp.c_str());
							pNetPack->port = conn->m_iConnPort;
							pNetPack->ilen = vecOutput[i].size();
							strncpy(pNetPack->ibuf, vecOutput[i].data(), pNetPack->ilen);
							pNetPack->ibuf[pNetPack->ilen] = 0x0;
							pNetPack->olen = 0;
							pNetPack->obuf[0] = 0x0;
							m_netPackCache.PutRecvQueue(pNetPack);
						}
						
						pNetPack = m_netPackCache.FreePack();
						pNetPack->sock = conn->m_ConnSock.GetSock();
						strcpy(pNetPack->ip, conn->m_strConnIp.c_str());
						pNetPack->port = conn->m_iConnPort;
						pNetPack->ilen = vecOutput[i].size();
						strncpy(pNetPack->ibuf, vecOutput[i].data(), pNetPack->ilen);
						pNetPack->ibuf[pNetPack->ilen] = 0x0;
						pNetPack->olen = 0;
						pNetPack->obuf[0] = 0x0;
						
						if (EndsWith(temp, m_endFlag))
						{
							//DEBUG_LOG("33333333333333333333333333333333333333333333");
							m_netPackCache.PutRecvQueue(pNetPack); //一个完整的包
						}
						else
						{
							//DEBUG_LOG("4444444444444444444444444444444444444444444444");
							m_netPackCache.AddContext(pNetPack->sock, pNetPack); //不完整的包，放入上下文
						}
					}
					else
					{
						if (EndsWith(temp, m_endFlag))
						{
							//DEBUG_LOG("55555555555555555555555555555555555555555555555");
							m_netPackCache.DelContext(pNetPack->sock);
							m_netPackCache.PutRecvQueue(pNetPack); //一个完整的包
						}
					}
				}
			}
		}
		
		return 0;
	}

	int CTcpNetServer::ReadOnePacket(CliConn* conn)
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
			DelCliConn(conn);
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
					DelCliConn(conn);
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
		
		CNetPack* pNetPack = m_netPackCache.FreePack();
		if (pNetPack)
		{
			pNetPack->sock = conn->m_ConnSock.GetSock();
			strcpy(pNetPack->ip, conn->m_strConnIp.c_str());
			pNetPack->port = conn->m_iConnPort;
			pNetPack->ilen = dataLen;
			strncpy(pNetPack->ibuf, buf, BUF_MAX_SIZE);
			pNetPack->ibuf[pNetPack->ilen] = 0x0;
			pNetPack->olen = 0;
			pNetPack->obuf[0] = 0x0;
			m_netPackCache.PutRecvQueue(pNetPack);
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
		
		DEBUG_LOG("END");
		return 0;
	}

	int CTcpNetServer::Readn(CliConn* conn, char* buf, int len)
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
					DelCliConn(conn);
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