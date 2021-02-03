#include "socket.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <fcntl.h>
#else
class WinNetBoot
{
public:
	WinNetBoot()
	{
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	}
	
	~WinNetBoot()
	{
		WSACleanup();
	}	
};
static WinNetBoot netBoot;
#endif


namespace ctm
{
	SOCKET_T Accept(SOCKET_T sockfd, string& outIp, int& outPort)
	{
		struct sockaddr_in m_sockAddrIn = { 0 };
		SOCKETLEN_T len = sizeof(m_sockAddrIn);
		SOCKET_T sock;
		while(1)
		{
			sock = accept(sockfd, (struct sockaddr *)&m_sockAddrIn, &len);
			if (SOCKET_INVALID == sock)
			{
                if (errno == EINTR) continue;
				return SOCKET_INVALID;
			}
			break;
		}
		outPort = ntohs(m_sockAddrIn.sin_port);
		outIp = inet_ntoa(m_sockAddrIn.sin_addr);

		return sock;
	}

	int Bind(SOCKET_T sockfd, const string& ip, int port)
	{
		struct sockaddr_in addr = {0};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = inet_addr(ip.c_str());
		if(SOCKET_ERR == bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)))
		{
			CloseSocket(sockfd);
			return -1;
		}

		return 0;
	}

	int Connect(SOCKET_T sockfd, const string& ip, int port)
	{
		struct sockaddr_in m_sockAddrIn = {0};
		m_sockAddrIn.sin_family = AF_INET;
		m_sockAddrIn.sin_port = htons(port);
		m_sockAddrIn.sin_addr.s_addr = inet_addr(ip.c_str());
		while(1)
		{
			if (SOCKET_ERR == connect(sockfd, (struct sockaddr*)&m_sockAddrIn, sizeof(m_sockAddrIn)))
			{
				if (errno == EINTR) continue;
				return -1;
			}
			break;
		}

		return 0;
	}

	int GetPeerName(SOCKET_T sockfd, string& outIp, int& outPort)
	{
		struct sockaddr_in m_sockAddrIn = {0};
		int len = sizeof(m_sockAddrIn);
		if (SOCKET_ERR == getpeername(sockfd, (struct sockaddr*)&m_sockAddrIn, (SOCKETLEN_T*)&len))
		{
			return -1;
		}
		outPort = ntohs(m_sockAddrIn.sin_port);
		outIp = inet_ntoa(m_sockAddrIn.sin_addr);

		return 0;
	}

	int GetSockName(SOCKET_T sockfd, string& outIp, int& outPort)
	{
		struct sockaddr_in m_sockAddrIn = {0};
		int len = sizeof(m_sockAddrIn);
		if (SOCKET_ERR == getsockname(sockfd, (struct sockaddr*)&m_sockAddrIn, (SOCKETLEN_T*)&len))
		{
			return -1;
		}
		outPort = ntohs(m_sockAddrIn.sin_port);
		outIp = inet_ntoa(m_sockAddrIn.sin_addr);

		return 0;
	}

	int GetTcpState(SOCKET_T sockfd)
	{
		int val = 0;
		int len = sizeof(val);
		if (getsockopt(sockfd, IPPROTO_TCP, TCP_INFO, &val, (SOCKETLEN_T*)&len) != 0)
		{
			return -1;
		}
		return  val;
	}

	int SetReuseAddr(SOCKET_T sockfd)
	{
		int val = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&val, sizeof(val)) != 0)
		{
			return -1;
		}

		val = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&val, sizeof(val)) != 0)
		{
			return -1;
		}
	
		return 0;
	}

	int SetNoDelay(SOCKET_T sockfd)
	{
		int val = 1;
		if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (const char*)&val, sizeof(val)) != 0)
		{
			return -1;
		}

		return 0;
	}

	int SetRcvLowat(SOCKET_T sockfd, int val)
	{
		return setsockopt(sockfd, SOL_SOCKET, SO_RCVLOWAT, (const char*)&val, sizeof(val));
	}

	int SetSndLowat(SOCKET_T sockfd, int val)
	{
		return setsockopt(sockfd, SOL_SOCKET, SO_SNDLOWAT, (const char*)&val, sizeof(val));
	}

	int GetRcvLowat(SOCKET_T sockfd)
	{
		int val = 0;
		SOCKETLEN_T len = sizeof(val);
		if (getsockopt(sockfd, SOL_SOCKET, SO_RCVLOWAT, &val, &len))
		{
			return -1;
		}

		return val;
	}

	int GetSndLowat(SOCKET_T sockfd)
	{
		int val = 0;
		SOCKETLEN_T len = sizeof(val);
		if (getsockopt(sockfd, SOL_SOCKET, SO_SNDLOWAT, &val, &len))
		{
			return -1;
		}
		
		return val;
	}

	std::string LocalHostName()
	{
		char hostName[256] = {0};
		if (gethostname(hostName, sizeof(hostName)) == -1)
		{
			return std::string("");
		}
		return std::string(hostName);
	}

	int GetHostIps(char* hostName, std::vector<std::string>& vecIps)
	{
		return Hostent2Ips(gethostbyname(hostName), vecIps);
	}

	int Hostent2Ips(struct hostent* htent, std::vector<std::string>& vecIps)
	{
		if (!htent)
		{
			return -1;
		}

		char buf[128] = {0};
		char** head = htent->h_addr_list;
		for(; *head; head++)
		{
			inet_ntop(htent->h_addrtype, *head, buf, sizeof(buf));
			vecIps.push_back(buf);
		}

		return 0;
	}

	int LocalHostIps(std::vector<std::string>& vecIps)
	{
		char hostName[256] = {0};
		if (gethostname(hostName, sizeof(hostName)) == -1)
		{
			return -1;
		}
		return Hostent2Ips(gethostbyname(hostName), vecIps);
	}

	bool SetKeepAlive(SOCKET_T sockfd, int interval)
	{
		int val = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (const char*)&val, sizeof(val)) == -1)
		{
			return false;
		}

		val = interval;
		if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPIDLE, (const char*)&val, sizeof(val)) == -1)
		{
			return false;
		}

		val = interval/3;
		if (val == 0) val = 1;
		if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPINTVL, (const char*)&val, sizeof(val)) == -1)
		{
			return false;
		}
		
		val = 3;
		if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPCNT, (const char*)&val, sizeof(val)) == -1)
		{
			return false;
		}

		return true;
	}

	SOCKET_T ListenSocket(const char* ip, const int port, const int num)
	{
		if(!ip) return SOCKET_INVALID;
		
		SOCKET_T fd = socket(AF_INET, SOCK_STREAM, 0);
		if(SOCKET_INVALID == fd)
		{ 
			return SOCKET_INVALID;
		}
		
		if (SetReuseAddr(fd) != 0)
		{
			CloseSocket(fd);
			return SOCKET_INVALID;
		}

		if(SOCKET_ERR == Bind(fd, ip, port))
		{
			CloseSocket(fd);
			return SOCKET_INVALID;
		}

		if(SOCKET_ERR == listen(fd, num))
		{
			CloseSocket(fd);
			return SOCKET_INVALID;
		}

		return fd;
	}

	bool NotFatalError(int err)
	{
		return (err == EINTR || err == EAGAIN || err == EWOULDBLOCK);
	}

	int ClearSockError(SOCKET_T sockfd)
	{
		int error = 0;
		SOCKETLEN_T len = sizeof(error);
		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*)&error, &len) != 0)
		{
			return SOCKET_INVALID;
		}
		return error;
	}

	int SetSockMode(SOCKET_T sockfd, bool bBlock)
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

	bool IsValidIp(const std::string& strIp)
	{
		if (strIp.size() < 7 || strIp.size() > 15) 
			return false;

		size_t begin = 0;
		size_t end = 0;
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

	int Read(SOCKET_T sockfd, char* buf, int len)
	{
		int retlen = 0;
		while(1)
		{
			retlen =  read(sockfd, buf, len);
			if (-1 == retlen)
			{
				if (errno == EINTR) continue;
				return -1;
			}

			break;
		}

		return retlen;
	}

	int Write(SOCKET_T sockfd, char* buf, int len)
	{
		int retlen = 0;
		while(1)
		{
			retlen =  write(sockfd, buf, len);
			if (-1 == retlen)
			{
				if (errno == EINTR) continue;
				return -1;
			}

			break;
		}

		return retlen;		
	}

	CSocket::CSocket(int sockType) :
		m_sock(SOCKET_INVALID),
		m_sockType(sockType),
		m_bindIp(""),
		m_bindPort(0),
		m_isListen(false),
		m_errno(0),
		m_errmsg("Success")
	{
		if (SOCK_STREAM == m_sockType)
			m_sock = socket(AF_INET, SOCK_STREAM, 0);
		else if(SOCK_DGRAM == m_sockType)
			m_sock = socket(AF_INET, SOCK_DGRAM, 0);

		if (!IsValid())
		{
			GetSystemError();
		}
	}

	CSocket::CSocket(SOCKET_T sockfd, int sockType) :
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
		m_errmsg(other.m_errmsg)
	{
	}

	CSocket& CSocket::operator=(const CSocket& other)
	{
		m_sock = other.m_sock;
		m_sockType = other.m_sockType;
		m_bindIp = other.m_bindIp;
		m_bindPort = other.m_bindPort;
		m_isListen = other.m_isListen;
		m_errno = other.m_errno;
		m_errmsg = other.m_errmsg;
		return *this;
	}

	bool CSocket::Bind(const char* ip, int port)
	{
		if (!IsValid()) 
			return false;
		if(SOCKET_ERR == ctm::Bind(m_sock, ip, port))
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
		
		if (SOCKET_ERR == listen(m_sock, backlog))
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
		if (SOCKET_ERR == ctm::GetPeerName(m_sock, outIp, outPort))
		{
			GetSystemError();
			return false;
		}
		return true;
	}

	bool CSocket::SetSockOpt(int level, int optname, const char* optval, int optlen)
	{
		if (!IsValid()) 
			return false;

		if (SOCKET_ERR == setsockopt(m_sock, level, optname, optval, optlen))
		{
			GetSystemError();
			return false;
		}

		return true;
	}

	bool CSocket::SetKeepAlive(int interval)
	{
		return ctm::SetKeepAlive(m_sock, interval);
	}

	CSocket CSocket::Accept(std::string& outIp, int& outPort)
	{
		if (!IsValid()) 
			return CSocket(SOCKET_INVALID, SOCK_STREAM);
		
		if (!m_isListen)
		{
			m_errno = ERR_NO_LISTEN;
			m_errmsg = "Socket no listen mode";
			return CSocket(SOCKET_INVALID, SOCK_STREAM);
		}
			
		return CSocket(ctm::Accept(m_sock, outIp, outPort), SOCK_STREAM);
	}

	bool CSocket::SetSockMode(bool bBlock)
	{
		if (!IsValid()) 
			return false;
		
		if (SOCKET_ERR == ctm::SetSockMode(m_sock, bBlock))
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
		
		if (SOCKET_ERR == shutdown(m_sock, how))
		{
			GetSystemError();
			return false;
		}

		return true;
	}

	bool CSocket::Connect(const char* ip, int port)
	{
		if (!IsValid()) 
			return false;
		if (SOCKET_ERR == ctm::Connect(m_sock, ip, port))
		{
			GetSystemError();
			return false;
		}
		return true;
	}

	int CSocket::Send(const char* buf, size_t len)
	{
		if (!IsValid()) 
			return SOCKET_ERR;
		
		int length = write(m_sock, buf, len);
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
			int length = write(m_sock, buf, len);
			if (length <= 0)
			{
				GetSystemError();
			}

			return length;
		}
		
		return SOCKET_ERR;
	}

	int CSocket::SendTo(const char* buf, size_t len, const std::string& dstIp, int dstPort)
	{
		if (!IsValid()) 
			return SOCKET_ERR;
		
		SetAddrZero();
		m_sockAddrIn.sin_family = AF_INET;
		m_sockAddrIn.sin_port = htons(dstPort);
		m_sockAddrIn.sin_addr.s_addr = inet_addr(dstIp.c_str());
		int length = sendto(m_sock, buf, len, 0, (struct sockaddr*)&m_sockAddrIn, sizeof(m_sockAddrIn));
		if (length <= 0)
		{
			GetSystemError();
		}

		return length;
	}

	int CSocket::Recv(char* buf, size_t len)
	{
		if (!IsValid()) 
			return SOCKET_ERR;
		
		int length = read(m_sock, buf, len);
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
			int length = read(m_sock, buf, len);
			if (length <= 0)
			{
				GetSystemError();
			}

			return length;
		}
		
		return SOCKET_ERR;
	}

	int CSocket::RecvFrom(char* buf, size_t len, std::string& srcIp, int& srcPort)
	{
		if (!IsValid()) 
			return SOCKET_ERR;
		
		SetAddrZero();
		SOCKETLEN_T addrLen = sizeof(m_sockAddrIn);
		int length = recvfrom(m_sock, buf, len, 0, (struct sockaddr*)&m_sockAddrIn, &addrLen);
		if (length <= 0)
		{
			GetSystemError();
		}
		
		return length;
	}

	bool CSocket::Renew()
	{
		Close();
		
		if (SOCK_STREAM == m_sockType)
			m_sock = socket(AF_INET, SOCK_STREAM, 0);
		else if(SOCK_DGRAM == m_sockType)
			m_sock = socket(AF_INET, SOCK_DGRAM, 0);

		if (m_sock == SOCKET_INVALID)
		{
			GetSystemError();
			return false;
		}

		// dup2(sock, m_sock);
		// CloseSocket(sock);
		
		return true;
	}

	bool CSocket::ReConnect()
	{
		if (!Renew()) return false;
		
		if (SOCKET_ERR == connect(m_sock, (struct sockaddr*)&m_sockAddrIn, sizeof(m_sockAddrIn)))
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
}



