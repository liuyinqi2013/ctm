#ifndef CTM_NET_SOCKET_H__
#define CTM_NET_SOCKET_H__

#ifdef WIN32
#include <Winsock2.h>
#define socket_t SOCKET
#define socklen_t int
#define sockbuf_t char
#define socket_invalid (SOCKET)(~0)
#define socket_error (-1)

inline int CloseSocket(socket_t& sockfd)
{
	return closesocket(sockfd);
}

#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#define socket_t int
#define socklen_t socklen_t
#define sockbuf_t void
#define socket_invalid (SOCKET)(~0)
#define socket_error (-1)

inline int CloseSocket(socket_t& sockfd)
{
	return close(sockfd);
}

#endif

inline socket_t CreateSocket(int domain, int type, int protocol)
{
	return socket(domain, type, protocol);
}

inline int ShutDown(socket_t& sockfd, int how)
{
	return shutdown(sockfd, how);
}

inline socket_t Accept(socket_t sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
	return accept(sockfd, addr, addrlen);
}

inline int Listen(socket_t sockfd, int backlog)
{
	return listen(sockfd, backlog);
}

inline int Connect(socket_t sockfd, const struct sockaddr* addr, socklen_t len)
{
	return connect(sockfd, addr, len);
}

inline int Send(socket_t sockfd, const sockbuf_t* buf, size_t len, int flags)
{
	return send(sockfd, buf, len, flags);
}

inline int Recv(socket_t sockfd, sockbuf_t* buf, size_t len, int flags) 
{
	return recv(sockfd, buf, len, flags);
}


#endif