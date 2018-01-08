#include "socket.h"
#include <string.h>
#include <stdio.h>
#ifndef WIN32
#include <fcntl.h>
#else
class NetBoot
{
public:
	NetBoot()
	{
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	}
	
	~NetBoot()
	{
		WSACleanup();
	}	
};
static NetBoot netBoot;
#endif


namespace ctm
{

	SOCKET_T ListenSocket(const char* ip, const int port, const int num)
	{
		if(!ip) return SOCKET_INVALID;
		
		SOCKET_T fd = Socket(AF_INET, SOCK_STREAM, 0);
		if(SOCKET_INVALID == fd) 
			return SOCKET_INVALID;

		struct sockaddr_in addr = {0};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = inet_addr(ip);
		if(SOCKET_ERR == Bind(fd, (struct sockaddr*)&addr, sizeof(addr)))
		{
			CloseSocket(fd);
			return SOCKET_INVALID;
		}

		if(SOCKET_ERR == Listen(fd, num))
		{
			CloseSocket(fd);
			return SOCKET_INVALID;
		}
		
		return fd;
	}

	int SetBlockMode(SOCKET_T sockfd, bool bBlock)
	{
#ifndef WIN32
		int flags;
	    if ((flags = fcntl(sockfd, F_GETFL)) == -1)          
			return SOCKET_OK; 
		
		if (!bBlock)        
			flags |= O_NONBLOCK;
		else        
			flags &= ~O_NONBLOCK; 
		
		if (fcntl(sockfd, F_SETFL, flags) == -1)               
			return SOCKET_ERR; 
#else
		u_long iMode = bBlock ? 0 : 1;
		if (ioctlsocket(sockfd, FIONBIO, &iMode) != NO_ERROR)
			return SOCKET_ERR;
#endif 
		return SOCKET_OK;
	}

	TcpClient::TcpClient() :
		m_tcpSock(SOCKET_INVALID),
		m_serverIp("127.0.0.1"),
		m_serverPort(0)
	{
		m_tcpSock = Socket(AF_INET, SOCK_STREAM, 0);
	}

	TcpClient::~TcpClient()
	{
		Close();
	}

	bool TcpClient::Connect(const std::string& serverIp, int serverPort)
	{
		m_serverIp = serverIp;
		m_serverPort = serverPort;
		if (SOCKET_INVALID == m_tcpSock)
		{
			return false;
		}

		struct sockaddr_in serverAddr = {0};
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(serverPort);
		serverAddr.sin_addr.s_addr = inet_addr(serverIp.c_str());
		if (ctm::Connect(m_tcpSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) != 0)
		{
			return false;
		}

		return true;
	}

	int TcpClient::Recv(char* buf, size_t len)
	{
		if(!buf)
		{
			return -1;
		}
		return ctm::Recv(m_tcpSock, buf, len, 0);
	}

	int TcpClient::Send(const char* buf, size_t len)
	{
		if(!buf)
		{
			return -1;
		}
		return ctm::Send(m_tcpSock, buf, len, 0);
	}

	void TcpClient::Close()
	{
		if (m_tcpSock != SOCKET_INVALID)
		{
			CloseSocket(m_tcpSock);
			m_tcpSock = SOCKET_INVALID;
		}
	}

	TcpServer::TcpServer(const std::string& ip, int port) : 
		m_tcpSock(SOCKET_INVALID),
		m_ip(ip),
		m_port(port)
	{
		m_tcpSock = ListenSocket(m_ip.c_str(), m_port, SOMAXCONN);
	}

	TcpServer::~TcpServer()
	{
		Close();
	}

	void TcpServer::Close()
	{
		if (m_tcpSock != SOCKET_INVALID)
		{
			CloseSocket(m_tcpSock);
			m_tcpSock = SOCKET_INVALID;
		}
	}

	void TcpServer::Run()
	{
		printf("Server Start\n");

		if(m_tcpSock == SOCKET_INVALID)
		{
			printf("m_tcpSock is SOCKET_INVALID!\n");
			return;
		}
		struct sockaddr_in clientAddr = {0};
		int len = sizeof(clientAddr);
		SOCKET_T clientSock = SOCKET_INVALID;
		while(1)
		{
			len = sizeof(clientAddr);
			memset(&clientAddr, 0, len);
			clientSock = Accept(m_tcpSock, (struct sockaddr*)&clientAddr, (SOCKETLEN_T*)&len);
			if(clientSock == SOCKET_INVALID)
			{
				printf("Accpcet failed!\n");
				continue;
			}

			printf("has a client connect : \n");
			printf("port : %d\n", ntohs(clientAddr.sin_port));
			printf("ip : %s\n", inet_ntoa(clientAddr.sin_addr));
			char *buf = "hello client";
			len = Send(clientSock, buf, strlen(buf), 0);
			printf("send buf : %s, len : %d\n", buf, len);
			printf("send len : %d\n", len);
			char rbuf[128];
			len =  Recv(clientSock, rbuf, 128, 0);
			rbuf[len] = '\0';
			printf("recv : %s\n", rbuf);
		}
		printf("Server stop\n");
	}
}



