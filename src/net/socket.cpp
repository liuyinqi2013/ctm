#include "socket.h"
#include <string.h>
#ifndef WIN32
#include <fcntl.h>
#endif


namespace ctm
{

	SOCKET_T ListenSocket(const char* ip, const int port, int listNum)
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

		if(SOCKET_ERR == Listen(fd, listNum))
		{
			CloseSocket(fd);
			return SOCKET_INVALID;
		}
		
		return fd;
	}

	int SetBlockMode(SOCKET_T sockfd, bool bBlock)
	{
		int flags;
	    if ((flags = fcntl(sockfd, F_GETFL)) == -1)          
			return SOCKET_OK; 
		
		if (bBlock)        
			flags |= O_NONBLOCK;
		else        
			flags &= ~O_NONBLOCK; 
		
		if (fcntl(sockfd, F_SETFL, flags) == -1)               
			return SOCKET_ERR; 
		
		return SOCKET_OK;
	}

}



