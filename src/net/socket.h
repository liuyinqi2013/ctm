#ifndef CTM_NET_SOCKET_H__
#define CTM_NET_SOCKET_H__
#include <string>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <map>
#include <vector>

#include "common/macro.h"
using std::string;

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
	enum TcpState
	{
		TCP_ESTABLISHED = 1,
		TCP_SYN_SENT,
		TCP_SYN_RECV,
		TCP_FIN_WAIT1,
		TCP_FIN_WAIT2,
		TCP_TIME_WAIT,
		TCP_CLOSE,
		TCP_CLOSE_WAIT,
		TCP_LAST_ACK,
		TCP_LISTEN,
		TCP_CLOSING
	};

	SOCKET_T Accept(SOCKET_T sockfd, string& outIp, int& outPort);

	int Connect(SOCKET_T sockfd, const string& ip, int port);

	int Bind(SOCKET_T sockfd, const string& ip, int port);

	int GetPeerName(SOCKET_T sockfd, string& outIp, int& outPort);

	int GetSockName(SOCKET_T sockfd, string& outIp, int& outPort);

	int GetTcpState(SOCKET_T sockfd);

	int SetReuseAddr(SOCKET_T sockfd);

	int SetNoDelay(SOCKET_T sockfd);

	int SetRcvLowat(SOCKET_T sockfd, int val);

	int SetSndLowat(SOCKET_T sockfd, int val);

	int GetRcvLowat(SOCKET_T sockfd);
	
	int GetSndLowat(SOCKET_T sockfd);

	std::string LocalHostName();

	int Hostent2Ips(struct hostent* htent, std::vector<std::string>& vecIps);

	int GetHostIps(char* hostName, std::vector<std::string>& vecIps);

	int LocalHostIps(std::vector<std::string>& vecIps);

	bool SetKeepAlive(SOCKET_T sockfd, int interval);

	SOCKET_T ListenSocket(const char* ip, const int port, const int num = SOMAXCONN);

	int SetSockMode(SOCKET_T sockfd, bool bBlock);

	inline int SetBlock(SOCKET_T sockfd) { return SetSockMode(sockfd, true); }

	inline int SetNonBlock(SOCKET_T sockfd) { return SetSockMode(sockfd, false); }

	bool NotFatalError(int err);

	bool IsValidIp(const std::string& strIp);

	int ClearSockError(SOCKET_T sockfd);

	int Read(SOCKET_T sockfd, char* buf, int len);

	int Write(SOCKET_T sockfd, char* buf, int len);

	class CSocket;

	bool operator==(const CSocket& lhs, const CSocket& rhs);
	
	class CSocket
	{
	public:
		
		enum SOCK_ERR_CODE
		{
			NO_ERR = 0,            // Ok
			ERR_SOCK_INVALID = 1,  // Invalid Sock
			ERR_IP_INVALID = 2,    // Invalid Ip
			ERR_PROT_INVALID = 3,  // Invalid Port
			ERR_NO_LISTEN = 4      // No listen
		};
				
		CSocket(int sockType = SOCK_STREAM);
		CSocket(SOCKET_T sockfd, int sockType);
		CSocket(const CSocket& other);
		
		virtual ~CSocket() {}

		CSocket& operator=(const CSocket& other);

		bool Bind(const char* ip, int port);
		
		bool Bind(const std::string& ip, int port)
		{
			return this->Bind(ip.c_str(), port);
		}

		bool Listen(int backlog);

		bool GetPeerName(std::string& outIp, int& outPort);

		bool SetSockOpt(int level, int optname, const char* optval, int  optlen);

		bool SetKeepAlive(int interval);

		CSocket Accept(std::string& outIp, int& outPort);

		bool SetSockMode(bool bBlock);

		bool SetNonBlock()
		{
			return SetSockMode(false);
		}

		bool SetBlock()
		{
			return SetSockMode(true);
		}

		bool ShutDown(int how);
		
		bool Connect(const char* ip, int port);

		bool Connect(const std::string& ip, int port)
		{
			return this->Connect(ip.c_str(), port);
		}

		int Send(const char* buf, size_t len);

		int Send(const std::string& strOut)
		{
			return this->Send(strOut.c_str(), strOut.size());
		}
		
		int SendEx(const char* buf, size_t len, struct timeval* timeOut);

		int SendTo(const char* buf, size_t len, const std::string& dstIp, int dstPort);

		int Recv(char* buf, size_t len);

		int RecvEx(char* buf, size_t len, struct timeval* timeOut);
		
		int RecvFrom(char* buf, size_t len, std::string& srcIp, int& srcPort);
		
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
	};

	inline bool operator==(const CSocket& lhs, const CSocket& rhs)
	{
		return (lhs.m_sock == rhs.m_sock);
	}
}

#endif
