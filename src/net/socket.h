#ifndef CTM_NET_SOCKET_H__
#define CTM_NET_SOCKET_H__

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#define socket_t int
#define INVALID_SOCKET -1;

inline socket_t CreateSocket(int domain, int type, int protocol)
{
	return socket(domain, type, protocol);
}

inline int CloseSocket(socket_t& sockfd)
{
	return close(sockfd);
}

inline int ShutDown(socket_t& sockfd, int how)
{
	return shutdown(sockfd, how)
}

inline socket_t Accept(socket_t sockfd, struct sockaddr* addr, int* addrlen)
{
	return accept(sockfd, addr, addrlen);
}

inline int Listen(socket_t sockfd, int backlog)
{
	return listen(sockfd, backlog);
}

inline int Recv(int sockfd, void *buf, size_t len, int flags)
{
	return recv(sockfd, buf, len, flags);
}

#endif