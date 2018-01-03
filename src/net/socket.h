#ifndef CTM_NET_SOCKET_H__
#define CTM_NET_SOCKET_H__

#ifdef WIN32
#include <Winsock2.h>
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

#pragma comment(lib, "ws2_32.lib")

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

#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

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

	SOCKET_T ListenSocket(const char* ip, const int port, int listNum);

	int SetBlockMode(SOCKET_T sockfd, bool bBlock);

	inline int SetBlock(SOCKET_T sockfd) { return SetBlockMode(sockfd, true); }

	inline int SetNonBlock(SOCKET_T sockfd) { return SetBlockMode(sockfd, false); }

}

#endif
