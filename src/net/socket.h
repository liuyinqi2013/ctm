#ifndef CTM_NET_SOCKET_H__
#define CTM_NET_SOCKET_H__
#include <string>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <map>

#include "common/refcount.h"
#include "common/macro.h"

#ifdef WIN32
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>

#pragma comment(lib, "ws2_32.lib")

#define SOCKET_T SOCKET
#define SOCKETLEN_T int
#define SOCKETBUF_T char
#define SOCKET_INVALID INVALID_SOCKET
#define SOCKET_ERR SOCKET_ERROR
#define SOCKET_OK  0

inline int CloseSocket(SOCKET_T& sockfd)
{
	return closesocket(sockfd);
}

inline int GetSockErrCode()
{
	return WSAGetLastError();
}

inline std::string GetSockErrMsg(int errCode)
{
	char buf[128] = {0};
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errCode, 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, sizeof(buf), NULL);
	return std::string(buf);
}

#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>


#define SOCKET_T int
#define SOCKETLEN_T socklen_t
#define SOCKETBUF_T void
#define SOCKET_INVALID -1
#define SOCKET_ERR -1
#define SOCKET_OK  0

inline int CloseSocket(SOCKET_T& sockfd)
{
	return close(sockfd);
}

inline int GetSockErrCode()
{
	return errno;
}

inline std::string GetSockErrMsg(int errCode)
{
	return strerror(errCode);
}

#endif

namespace ctm
{
	inline SOCKET_T Socket(int domain, int type, int protocol)
	{
		return socket(domain, type, protocol);
	}

	inline int ShutDown(SOCKET_T& sockfd, int how)
	{
		return shutdown(sockfd, how);
	}

	inline SOCKET_T Accept(SOCKET_T sockfd, struct sockaddr* addr, SOCKETLEN_T* addrlen)
	{
		return accept(sockfd, addr, addrlen);
	}
	SOCKET_T Accept(SOCKET_T sockfd, string& outIp, int& outPort);

	inline int Bind(SOCKET_T sockfd, struct sockaddr* addr, SOCKETLEN_T addrlen)
	{
		return bind(sockfd, addr, addrlen);
	}
	int Bind(SOCKET_T sockfd, const string& ip, int port);

	inline int Listen(SOCKET_T sockfd, int backlog)
	{
		return listen(sockfd, backlog);
	}

	inline int SetSockOpt(SOCKET_T sockfd, int level, int optname, const char* optval, int  optlen)
	{
		return setsockopt(sockfd, level, optname, optval, optlen);
	}

	inline int GetPeerName(SOCKET_T sockfd, struct sockaddr* addr, SOCKETLEN_T* len)
	{
		return getpeername(sockfd, addr, len);
	}
	int GetPeerName(SOCKET_T sockfd, string& outIp, int& outPort);

	inline struct hostent* GetHostByName(const char *name)
	{
		return gethostbyname(name);
	}
	
	inline int Connect(SOCKET_T sockfd, const struct sockaddr* addr, SOCKETLEN_T len)
	{
		return connect(sockfd, addr, len);
	}
	int Connect(SOCKET_T sockfd, const string& ip, int port);

	inline int Send(SOCKET_T sockfd, const SOCKETBUF_T* buf, size_t len, int flags)
	{
		return send(sockfd, buf, len, flags);
	}

	inline int SendTo(SOCKET_T sockfd, const SOCKETBUF_T* buf, size_t len, int flags, 
		const struct sockaddr* dest_addr, SOCKETLEN_T addrlen)
	{
		return sendto(sockfd, buf, len, flags, dest_addr, addrlen);
	}

	inline int Recv(SOCKET_T sockfd, SOCKETBUF_T* buf, size_t len, int flags) 
	{
		return recv(sockfd, buf, len, flags);
	}

	inline int RecvFrom(SOCKET_T sockfd, SOCKETBUF_T* buf, size_t len, int flags,
	                        struct sockaddr* src_addr, SOCKETLEN_T* addrlen)
	{
		return recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
	}

	bool SetKeepAlive(SOCKET_T sockfd, int interval);

	SOCKET_T ListenSocket(const char* ip, const int port, const int num = SOMAXCONN);

	int SetBlockMode(SOCKET_T sockfd, bool bBlock);

	inline int SetBlock(SOCKET_T sockfd) { return SetBlockMode(sockfd, true); }

	inline int SetNonBlock(SOCKET_T sockfd) { return SetBlockMode(sockfd, false); }

	bool NotFatalError(int err);

	class CSocket;
	bool operator==(const CSocket& lhs, const CSocket& rhs);
	
	class CSocket
	{
	public:
		
		typedef enum sock_type
		{
			SOCK_TYPE_STREAM = 0, // TCP
			SOCK_TYPE_DGRAM = 1	  // UDP
		} SOCK_TYPE;

		typedef enum sock_error
		{
			NO_ERR = 0, // No error
			ERR_SOCK_INVALID = 1, //
			ERR_IP_INVALID = 2, //
			ERR_PROT_INVALID = 3, //
			ERR_NO_LISTEN = 4   // No listen
		} SOCK_ERR_CODE;
				
		CSocket(SOCK_TYPE sockType = SOCK_TYPE_STREAM);
		CSocket(SOCKET_T sockfd, SOCK_TYPE sockType);
		CSocket(const CSocket& other);
		
		virtual ~CSocket() 
		{
			/*
			if (m_refCount.Only()) 
			{
				Close();
			}
			*/
		}

		CSocket& operator=(const CSocket& other);

		bool Bind(const char* ip, const int& port);
		
		bool Bind(const std::string& ip, const int& port)
		{
			return this->Bind(ip.c_str(), port);
		}

		bool Listen(int backlog);

		bool GetPeerName(std::string& outIp, int& outPort);

		bool SetSockOpt(int level, int optname, const char* optval, int  optlen);

		bool SetKeepAlive(int interval);

		CSocket Accept(std::string& outIp, int& outPort);

		bool SetBlockMode(bool bBlock);

		bool SetNonBlock()
		{
			return SetBlockMode(false);
		}

		bool SetBlock()
		{
			return SetBlockMode(true);
		}

		bool ShutDown(int how);
		
		bool Connect(const char* ip, const int& port);
		bool Connect(const std::string& ip, const int& port)
		{
			return this->Connect(ip.c_str(), port);
		}
		
		int Send(const char* buf, size_t len, int flags = 0);
		
		int Send(const std::string& strBuf, int flags = 0)
		{
			return this->Send(strBuf.data(), strBuf.size(), flags);
		}

		int SendEx(const char* buf, size_t len, struct timeval* timeOut);

		int SendEx(const std::string& strBuf, struct timeval* timeOut)
		{
			return this->SendEx(strBuf.data(), strBuf.size(), timeOut);
		}
		
		int SendTo(const char* buf, size_t len, const std::string& dstIp, const int& dstPort, int flags = 0);
		int SendTo(const std::string& strBuf, const std::string& dstIp, const int& dstPort, int flags = 0)
		{
			return this->SendTo(strBuf.data(), strBuf.size(), dstIp, dstPort, flags);
		}

		int Recv(char* buf, size_t len, int flags = 0);
		int Recv(std::string& strOut, int flags = 0);

		int RecvEx(char* buf, size_t len, struct timeval* timeOut);
		
		int RecvFrom(char* buf, size_t len, std::string& srcIp, int& srcPort, int flags = 0);
		int RecvFrom(std::string& strOut, std::string& srcIp, int& srcPort, int flags = 0);
		
		void Close()
		{
			if (IsValid()) { CloseSocket(m_sock); m_sock = SOCKET_INVALID; }
		}

		bool IsListen() const
		{
			return m_isListen;
		}

		bool IsValid() const
		{
			return (m_sock != SOCKET_INVALID);
		}

		int GetErrCode() const
		{
			return m_errno;
		}

		std::string GetErrMsg() const
		{
			return m_errmsg;
		}

		bool Compare(const SOCKET_T sockfd)
		{
			return (m_sock == sockfd);
		}
		
		bool Compare(const CSocket& other)
		{
			return (m_sock == other.m_sock);
		}

		SOCKET_T GetSock() const
		{
			return m_sock;
		}

		std::string GetBindIp() const
		{
			return m_bindIp;
		}

		int GetBindPort() const
		{
			return m_bindPort;
		}

		bool Renew();

		bool ReConnect();
		
		bool ReConnect(const char* ip, const int& port);
		
	protected:
		
		void SetAddrZero()
		{
			memset(&m_sockAddrIn, 0, sizeof(m_sockAddrIn));
		}

	private:
		
		void GetSystemError()
		{
			m_errno = GetSockErrCode();
			m_errmsg = GetSockErrMsg(m_errno);
		}

		friend bool operator==(const CSocket& lhs, const CSocket& rhs);
		
	private:
		SOCKET_T m_sock;
		int m_sockType;
		std::string m_bindIp;
		int m_bindPort;
		bool m_isListen;
		int m_errno;
		std::string m_errmsg;
		struct sockaddr_in m_sockAddrIn;
		CRefCount m_refCount;
	};

	inline bool operator==(const CSocket& lhs, const CSocket& rhs)
	{
		return (lhs.m_sock == rhs.m_sock);
	}

	bool IsValidIp(const std::string& strIp);

	class TcpClient
	{
	public:
		TcpClient();
		virtual ~TcpClient();
		bool Connect(const std::string& serverIp, int serverPort);
		int Recv(char* buf, size_t len);
		int Recv(std::string& strBuf);
		int Send(const char* buf, size_t len);
		int Send(const std::string& strBuf);

		SOCKET_T GetSocket() const
		{
			return m_tcpSock.GetSock();
		}

		int GetErrCode() const
		{
			return m_tcpSock.GetErrCode();;
		}
				
		std::string GetErrMsg() const
		{
			return m_tcpSock.GetErrMsg();
		}

		bool SetNonBlock()
		{
			return m_tcpSock.SetNonBlock();
		}
		
	public:
		CSocket m_tcpSock;
		std::string m_serverIp;
		int m_serverPort;
	};
}

#endif
