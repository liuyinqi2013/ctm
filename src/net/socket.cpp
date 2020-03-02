#include "socket.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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
		
		int val = 1;
		if (!SetSockOpt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&val, sizeof(val)))
		{
			return SOCKET_INVALID;
		}
		
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


	CSocket::CSocket(SOCK_TYPE sockType) :
		m_sock(SOCKET_INVALID),
		m_sockType(sockType),
		m_bindIp(""),
		m_bindPort(0),
		m_isListen(false),
		m_errno(0),
		m_errmsg("Success")
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

	CSocket::CSocket(SOCKET_T sockfd, SOCK_TYPE sockType) :
		m_sock(sockfd),
		m_sockType(sockType),
		m_errno(0),
		m_errmsg("Success")
	{
	}

	CSocket::CSocket(const CSocket& other) :
		m_sock(other.m_sock),
		m_sockType(other.m_sockType),
		m_bindIp(other.m_bindIp),
		m_bindPort(other.m_bindPort),
		m_isListen(other.m_isListen),
		m_errno(other.m_errno),
		m_errmsg(other.m_errmsg),
		m_refCount(other.m_refCount)
	{
	}

	CSocket& CSocket::operator=(const CSocket& other)
	{
		DEBUG_LOG();
		if (m_sock != other.m_sock)
		{
			if(m_refCount.Only())
			{
				Close();
			}
			m_sock = other.m_sock;
			m_refCount = other.m_refCount;
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
		if (!IsValid()) 
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
		if (SOCKET_ERR == ctm::GetPeerName(m_sock, (struct sockaddr*)&m_sockAddrIn, (SOCKETLEN_T*)&len))
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

	bool CSocket::SetKeepAlive(int interval)
	{
		int val = 1;
		if (!SetSockOpt(SOL_SOCKET, SO_KEEPALIVE, (const char*)&val, sizeof(val)))
		{
			return false;
		}

		val = interval;
		if (!SetSockOpt(IPPROTO_TCP, TCP_KEEPIDLE, (const char*)&val, sizeof(val)))
		{
			return false;
		}

		val = interval/3;
		if (val == 0) val = 1;
		if (!SetSockOpt(IPPROTO_TCP, TCP_KEEPINTVL, (const char*)&val, sizeof(val)))
		{
			return false;
		}

		val = 3;
		if (!SetSockOpt(IPPROTO_TCP, TCP_KEEPCNT, (const char*)&val, sizeof(val)))
		{
			return false;
		}

		return true;
		
	}

	CSocket CSocket::Accept(std::string& outIp, int& outPort)
	{
		if (!IsValid()) 
			return CSocket(SOCKET_INVALID, SOCK_TYPE_STREAM);
		
		if (!m_isListen)
		{
			m_errno = ERR_NO_LISTEN;
			m_errmsg = "Socket no listen mode";
			return CSocket(SOCKET_INVALID, SOCK_TYPE_STREAM);
		}

		SetAddrZero();
		SOCKETLEN_T len = sizeof(m_sockAddrIn);
		SOCKET_T s = ctm::Accept(m_sock, (struct sockaddr*)&m_sockAddrIn, &len);
		if (SOCKET_INVALID == s)
		{
			GetSystemError();
		}
		outPort = ntohs(m_sockAddrIn.sin_port);
		outIp = inet_ntoa(m_sockAddrIn.sin_addr);
			
		return CSocket(s, SOCK_TYPE_STREAM);
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

	bool CSocket::ShutDown(int how)
	{
		if (!IsValid()) 
			return false;
		
		if (SOCKET_ERR == ctm::ShutDown(m_sock, how))
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
		if (length <= 0)
		{
			GetSystemError();
		}

		return length;
	}

	int CSocket::SendEx(const char* buf, size_t len, struct timeval* timeOut)
	{
		if (!IsValid()) 
			return SOCKET_ERR;
		
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(m_sock, &fds);
		
		int iRet = select(m_sock + 1, NULL, &fds, NULL, timeOut);
		if (iRet == 0)
		{
			return 0;
		}
		else if (iRet > 0 && FD_ISSET(m_sock, &fds))
		{
			int length = ctm::Send(m_sock, buf, len, 0);
			if (length <= 0)
			{
				GetSystemError();
			}

			return length;
		}
		
		return SOCKET_ERR;
	}

	int CSocket::SendTo(const char* buf, size_t len, const std::string& dstIp, const int& dstPort, int flags)
	{
		if (!IsValid()) 
			return SOCKET_ERR;
		
		SetAddrZero();
		m_sockAddrIn.sin_family = AF_INET;
		m_sockAddrIn.sin_port = htons(dstPort);
		m_sockAddrIn.sin_addr.s_addr = inet_addr(dstIp.c_str());
		int length = ctm::SendTo(m_sock, buf, len, flags, (struct sockaddr*)&m_sockAddrIn, sizeof(m_sockAddrIn));
		if (length <= 0)
		{
			GetSystemError();
		}

		return length;
	}

	int CSocket::Recv(char* buf, size_t len, int flags)
	{
		if (!IsValid()) 
			return SOCKET_ERR;
		
		int length = ctm::Recv(m_sock, buf, len, flags);
		if (length <= 0)
		{
			GetSystemError();
		}
		
		return length;
	}

	int CSocket::RecvEx(char* buf, size_t len, struct timeval* timeOut)
	{
		if (!IsValid()) 
			return SOCKET_ERR;
		
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(m_sock, &fds);
		
		int iRet = select(m_sock + 1,  &fds, NULL, NULL, timeOut);
		if (iRet == 0)
		{
			return 0;
		}
		else if (iRet > 0 && FD_ISSET(m_sock, &fds))
		{
			int length = ctm::Recv(m_sock, buf, len, 0);
			if (length <= 0)
			{
				GetSystemError();
			}

			return length;
		}
		
		return SOCKET_ERR;

	}

	int CSocket::RecvFrom(char* buf, size_t len, std::string& srcIp, int& srcPort, int flags)
	{
		if (!IsValid()) 
			return SOCKET_ERR;
		
		SetAddrZero();
		SOCKETLEN_T addrLen = sizeof(m_sockAddrIn);
		int length = ctm::RecvFrom(m_sock, buf, len, flags, (struct sockaddr*)&m_sockAddrIn, &addrLen);
		if (length <= 0)
		{
			GetSystemError();
		}
		
		return length;
		
	}

	bool CSocket::Renew()
	{
		SOCKET_T sock;
		
		if (SOCK_TYPE_STREAM == m_sockType)
			sock = Socket(AF_INET, SOCK_STREAM, 0);
		else if(SOCK_TYPE_DGRAM == m_sockType)
			sock = Socket(AF_INET, SOCK_DGRAM, 0);
		if (sock == SOCKET_INVALID)
		{
			GetSystemError();
			return false;
		}

		dup2(sock, m_sock);
		CloseSocket(sock);
		
		return true;
	}

	bool CSocket::ReConnect()
	{
		if (!Renew()) return false;
		
		if (SOCKET_ERR == ctm::Connect(m_sock, (struct sockaddr*)&m_sockAddrIn, sizeof(m_sockAddrIn)))
		{
			GetSystemError();
			return false;
		}
		
		return true;
	}
		
	bool CSocket::ReConnect(const char* ip, const int& port)
	{
		if (!Renew()) return false;

		return Connect(ip, port);
	}

	bool IsValidIp(const std::string& strIp)
	{
		if (strIp.size() < 7 || strIp.size() > 15) 
			return false;

		int begin = 0;
		int end = 0;
		int cnt = 0;
		int num = 0;
		while((end = strIp.find(".", begin)) != strIp.npos)
		{
			if (end == begin) 
				return false;
			
			num = atoi(strIp.substr(begin, end - begin).c_str());
			if (num < 0 || num > 255)
				return false;
			
			++cnt;
			begin = end + 1;
		}

		if (cnt != 3)
			return false;
		
		num = atoi(strIp.substr(begin).c_str());
		if (num < 0 || num > 255)
			return false;

		return true;
	}

	TcpClient::TcpClient() :
		m_serverIp("127.0.0.1"),
		m_serverPort(0)
	{
		
	}

	TcpClient::~TcpClient()
	{
	}

	bool TcpClient::Connect(const std::string& serverIp, int serverPort)
	{
		return m_tcpSock.Connect(serverIp, serverPort);
	}

	int TcpClient::Recv(char* buf, size_t len)
	{
		if(!buf)
		{
			return -1;
		}
		return m_tcpSock.Recv(buf, len);
	}

	int TcpClient::Send(const char* buf, size_t len)
	{
		if(!buf)
		{
			return -1;
		}
		return m_tcpSock.Send(buf, len);
	}
}



