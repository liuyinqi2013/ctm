#include "tcpserver.h"
#include <sys/epoll.h>

TcpServer::TcpServer(const std::string& ip, int port) : 
	m_ip(ip),
	m_port(port)
{
}

TcpServer::~TcpServer()
{

}

void TcpServer::Run()
{
	DEBUG_LOG("Server Start");

	if(!m_tcpSock.IsValid())
	{
		ERROR_LOG("m_tcpSock INVALID!");
		return;
	}

	if(!m_tcpSock.Bind(m_ip, m_port))
	{
		ERROR_LOG("errcode = %d, errmsg = %s!", m_tcpSock.GetErrCode(), m_tcpSock.GetErrMsg().c_str());
		return;
	}

	if(!m_tcpSock.Listen(SOMAXCONN))
	{
		ERROR_LOG("errcode = %d, errmsg = %s!", m_tcpSock.GetErrCode(), m_tcpSock.GetErrMsg().c_str());
		return;
	}


	int m_epollFd = epoll_create(1024);
	if (-1 == m_epollFd)
	{
		ERROR_LOG("epoll_create1 failed");
		return ;
	}
	struct epoll_event event = {0};
	event.events  = EPOLLIN;
	event.data.fd = m_tcpSock.GetSock();

	int iRet = epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_tcpSock.GetSock(), &event);
	if (iRet != 0)
	{
		ERROR_LOG("epoll_ctl failed");
		return ;
	}

	int clientPort;
	std::string clientIp;
	int len = 0;
	struct epoll_event events[1024];
	while(1)
	{
		int nfds = epoll_wait(m_epollFd, events, 1024, -1);
		if (nfds == -1) 
		{
			ERROR_LOG("epoll_wait failed");
			return;
		}
		DEBUG_LOG("reader nfds = %d", nfds);

		for (int i = 0; i < nfds; ++i)
		{
			if (events[i].data.fd == m_tcpSock.GetSock())
			{
				CSocket clientSock = m_tcpSock.Accept(clientIp, clientPort);
				if(!clientSock.IsValid())
				{
					ERROR_LOG("errcode = %d, errmsg = %s!", m_tcpSock.GetErrCode(), m_tcpSock.GetErrMsg().c_str());
					continue;
				}

				clientSock.GetPeerName(clientIp, clientPort);
				DEBUG_LOG("has a client connect : ");
				DEBUG_LOG("port : %d", clientPort);
				DEBUG_LOG("ip : %s", clientIp.c_str());
				char *buf = "God>";
				len = clientSock.Send(buf, strlen(buf));
				if (len < 0)
				{
					ERROR_LOG("errcode = %d, errmsg = %s!", clientSock.GetErrCode(), clientSock.GetErrMsg().c_str());
				}
				DEBUG_LOG("send buf : %s, len : %d", buf, len);

				clientSock.SetNonBlock();
				struct epoll_event event = {0};
				event.events  = EPOLLIN | EPOLLET;
				event.data.fd = clientSock.GetSock();
			
				int iRet = epoll_ctl(m_epollFd, EPOLL_CTL_ADD, clientSock.GetSock(), &event);
				if (iRet != 0)
				{
					ERROR_LOG("epoll_ctl failed");
				}
				m_sockClients[clientSock.GetSock()] = clientSock; 
			}
			else if (events[i].events & EPOLLIN)
			{
				while(1)
				{
					char rbuf[1024 + 1] = {0};
					DEBUG_LOG("-----------------------------------------------");
					CSocket clientSock = m_sockClients[events[i].data.fd];
					len = clientSock.Recv(rbuf, 64);
					DEBUG_LOG("recv = %s, len = %d", rbuf, len);
					if (len <= 0)
					{	
						DEBUG_LOG("errcode = %d, errmsg = %s!", clientSock.GetErrCode(), clientSock.GetErrMsg().c_str());
						if ((clientSock.GetErrCode() == EWOULDBLOCK || clientSock.GetErrCode() == EAGAIN)) //需要等待资源 
						{
							DEBUG_LOG("need wait");
						}
						else
						{
							m_sockClients.erase(events[i].data.fd);
							int iRet = epoll_ctl(m_epollFd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
							if (iRet != 0)
							{
								ERROR_LOG("epoll_ctl failed");
							}
						}
						break;
					}

					if (strstr(rbuf, "@@"))
					{
						char *buf = "God>";
						len = clientSock.Send(buf, strlen(buf));
					}

				}
			}
		}
	}
	DEBUG_LOG("Server stop");
}


