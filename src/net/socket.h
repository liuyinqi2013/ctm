#ifndef CTM_NET_SOCKET_H__
#define CTM_NET_SOCKET_H__
#include <string>
#include <stdio.h>
#include <errno.h>

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

inline int GetLastSockErrCode()
{
	return WSAGetLastError();
}

inline std::string StrSockErrMsg(int errCode)
{
	return std::string("");
}

#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

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

inline int GetLastSockErrCode()
{
	return errno;
}

inline std::string StrSockErrMsg(int errCode)
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

	inline int Bind(SOCKET_T sockfd, struct sockaddr* addr, SOCKETLEN_T addrlen)
	{
		return bind(sockfd, addr, addrlen);
	}

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

	struct hostent* GetHostByName(const char *name)
	{
		return gethostbyname(name);
	}
	
	inline int Connect(SOCKET_T sockfd, const struct sockaddr* addr, SOCKETLEN_T len)
	{
		return connect(sockfd, addr, len);
	}

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

	SOCKET_T ListenSocket(const char* ip, const int port, const int num = SOMAXCONN);

	int SetBlockMode(SOCKET_T sockfd, bool bBlock);

	inline int SetBlock(SOCKET_T sockfd) { return SetBlockMode(sockfd, true); }

	inline int SetNonBlock(SOCKET_T sockfd) { return SetBlockMode(sockfd, false); }

	class CSocket;
	
	class CSocket
	{
	public:
		typedef enum sock_type
		{
			SOCK_TYPE_STREAM = 0, // TCP
			SOCK_TYPE_DGRAM = 1	  // UDP
		} SOCK_TYPE;
		
		CSocket(int sockType = SOCK_TYPE_STREAM);
		CSocket(SOCKET_T sockfd);
		CSocket(const CSocket& other);
		
		virtual ~CSocket() 
		{
			Close();
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

		CSocket Accept(std::string& outIp, int& outPort);
		
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
		
		int SendTo(const char* buf, size_t len, const std::string& dstIp, const int& dstPort, int flags = 0);
		int SendTo(const std::string& strBuf, const std::string& dstIp, const int& dstPort, int flags = 0)
		{
			return this->SendTo(strBuf.data(), strBuf.size(), dstIp, dstPort, flags);
		}

		int Recv(char* buf, size_t len, int flags = 0);
		int Recv(std::string& strOut, int flags = 0);
		int RecvFrom(char* buf, size_t len, int flags = 0);
		int RecvFrom(std::string& strOut, int flags = 0);
		int RecvFrom(char* buf, size_t len, std::string& srcIp, int& srcPort, int flags = 0);
		int RecvFrom(std::string& strOut, std::string& srcIp, int& srcPort, int flags = 0);
		
		void Close()
		{
			if (IsValid()) CloseSocket(m_sock);
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

		const std::string& GetErrMsg() const
		{
			return m_errmsg;
		}

		bool Compare(const SOCKET_T sockfd)
		{
			return (m_sock == sockfd);
		}
		
		bool Compare(const CSocket& other)
		{
			return (m_sock == other.m_sock)
		}

	protected:
		void SetAddrZero();
		{
			memset(&m_sockAddrIn, 0, sizeof(m_sockAddrIn));
		}
	private:
		SOCKET_T m_sock;
		int m_sockType;
		std::string m_bindIp;
		int m_bindPort;
		bool m_isListen;
		int m_errno;
		std::string m_errmsg;
		struct sockaddr_in m_sockAddrIn;
	};

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

		bool ShutDownRecv()
		{
			return !(ctm::ShutDown(m_tcpSock, 0));
		}

		bool ShutDownSend()
		{
			return !(ctm::ShutDown(m_tcpSock, 1));
		}

		bool ShutDown()
		{
			return !(ctm::ShutDown(m_tcpSock, 2));
		}

		SOCKET_T GetSocket() const
		{
			return m_tcpSock;
		}

	private:
		void Close();
	private:
		SOCKET_T m_tcpSock;
		std::string m_serverIp;
		int m_serverPort;
	};

	class TcpServer
	{
	public:
		TcpServer(const std::string& ip, int port);
		virtual ~TcpServer();
		void Run();
	private:
		void Close();
	private:
		SOCKET_T m_tcpSock;
		std::string m_ip;
		int m_port;
	};

}

#endif
