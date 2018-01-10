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


	CSocket::CSocket(int sockType) :
		m_sock(SOCKET_INVALID),
		m_sockType(sockType),
		m_bindIp(""),
		m_bindPort(0),
		m_isListen(false),
		m_errno(0),
		m_errmsg("")
	{
		if (SOCK_TYPE_STREAM == sockType)
			m_sock = Socket(AF_INET, SOCK_STREAM, 0);
		else if(SOCK_TYPE_DGRAM == sockType)
			m_sock = Socket(AF_INET, SOCK_DGRAM, 0);
		
		if (!IsValid())
		{
			GetSystemError();
		}		
	}

	CSocket::CSocket(SOCKET_T sockfd) :
		m_sock(sockfd),
		m_sockType(SOCK_TYPE_STREAM),
		m_bindIp(""),
		m_bindPort(0),
		m_isListen(false),
		m_errno(0),
		m_errmsg("")
	{
	}

	CSocket::CSocket(const CSocket& other) :
		m_sock(other.m_sock),
		m_sockType(other.m_sockType),
		m_bindIp(other.m_bindIp),
		m_bindPort(other.m_bindPort),
		m_isListen(other.m_isListen),
		m_errno(other.m_errno),
		m_errmsg(other.m_errmsg)
	{
	
	}

	CSocket& CSocket::operator=(const CSocket& other)
	{
		if (m_sock != other.m_sock)
		{
			m_sock = other.m_sock;
			m_sockType = other.m_sockType;
			m_bindIp = other.m_bindIp;
			m_bindPort = other.m_bindPort;
			m_isListen = other.m_isListen;
			m_errno = other.m_errno;
			m_errmsg = other.m_errmsg;
		}

		return *this;
	}

	bool CSocket::Bind(const char* ip, const int& port)
	{
		if (!IsValid() || !ip || port <= 0) 
			return false;
		SetAddrZero();
		m_sockAddrIn.sin_family = AF_INET;
		m_sockAddrIn.sin_port = htons(port);
		m_sockAddrIn.sin_addr.s_addr = inet_addr(ip);
		if(SOCKET_ERR == ctm::Bind(m_sock, (struct sockaddr*)&m_sockAddrIn, sizeof(m_sockAddrIn)))
		{
			GetSystemError();
			return false;
		}
		
		m_bindIp = ip;
		m_bindPort = port;
		
		return true;
	}

	bool CSocket::Listen(int backlog)
	{
		if (!IsValid() || backlog <= 0) 
			return false;
		
		if (SOCKET_ERR == ctm::Listen(m_sock, backlog))
		{
			GetSystemError();
			return false;
		}
		m_isListen = true;
		
		return true;
	}
	
	bool CSocket::GetPeerName(std::string& outIp, int& outPort)
	{
		if (!IsValid()) 
			return false;
		SetAddrZero();
		int len = sizeof(m_sockAddrIn);
		if (SOCKET_ERR == ctm::GetPeerName(m_sock, (struct sockaddr*)&m_sockAddrIn, &len))
		{
			GetSystemError();
			return false;
		}
		outPort = ntohs(m_sockAddrIn.sin_port);
		outIp = inet_ntoa(m_sockAddrIn.sin_addr);
		return true;
	}

	bool CSocket::SetSockOpt(int level, int optname, const char* optval, int optlen)
	{
		if (!IsValid()) 
			return false;

		if (SOCKET_ERR == ctm::SetSockOpt(m_sock, level, optname, optval, optlen))
		{
			GetSystemError();
			return false;
		}

		return true;
	}

	CSocket CSocket::Accept(std::string& outIp, int& outPort)
	{
		if (!IsValid()) 
			return CSocket(SOCKET_INVALID);

		SetAddrZero();
		int len = sizeof(m_sockAddrIn);
		SOCKET_T s = ctm::Accept(m_sock, (struct sockaddr*)&m_sockAddrIn, &len);
		if (SOCKET_INVALID == s)
		{
			GetSystemError();
		}

		return CSocket(s);
	}

	bool CSocket::SetBlockMode(bool bBlock)
	{
		if (!IsValid()) 
			return false;
		
		if (SOCKET_ERR == ctm::SetBlockMode(m_sock, bBlock))
		{
			GetSystemError();
			return false;
		}

		return true;
	}

	bool CSocket::Connect(const char* ip, const int& port)
	{
		if (!IsValid()) 
			return false;

		SetAddrZero();
		m_sockAddrIn.sin_family = AF_INET;
		m_sockAddrIn.sin_port = htons(port);
		m_sockAddrIn.sin_addr.s_addr = inet_addr(ip);
		if (SOCKET_ERR == ctm::Connect(m_sock, (struct sockaddr*)&m_sockAddrIn, sizeof(m_sockAddrIn)))
		{
			GetSystemError();
			return false;
		}

		return true;
	}

	int CSocket::Send(const char* buf, size_t len, int flags)
	{
		if (!IsValid()) 
			return SOCKET_ERR;
		
		int length = ctm::Send(m_sock, buf, len, flags);
		if (SOCKET_ERR == length)
		{
			GetSystemError();
		}

		return length;
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



