#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include <sys/time.h>
#include <sys/stat.h>

#include "common/log.h"
#include "socket.h"


namespace ctm {

	int Accept(int sockfd, struct sockaddr_in *addr)
	{
		int fd;
		socklen_t len = sizeof(struct sockaddr_in);
		while(1) {
			fd = accept(sockfd, (struct sockaddr *)&addr, &len);
			if (fd < 0 && errno == EINTR) {
				continue;
			}
			break;
		}

		return fd;
	}

	int BindIPv4(int sockfd, const char *ip, int port)
	{
		struct sockaddr_in addr = {0};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		if (inet_pton(AF_INET, ip, &addr.sin_addr.s_addr) < 0) {
			return -1;
		}
		return bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
	}

	int BindIPv6(int sockfd, const char *ip, int port)
	{
		struct sockaddr_in6 addr6 = {0};
		addr6.sin6_family = AF_INET6;
		addr6.sin6_port = htons(port);
		if (inet_pton(AF_INET6, ip, &addr6.sin6_addr.s6_addr) < 0) {
				return -1;
		}
		return bind(sockfd, (struct sockaddr*)&addr6, sizeof(addr6));
	}

	int Bind(int sockfd, int family, const char *ip, int port)
	{
		if (family == AF_INET6)
			return BindIPv6(sockfd, ip, port);

		return BindIPv4(sockfd, ip, port);
	}

	int ListenIPv4(const char* ip, int port, int num) 
	{
		return Listen(AF_INET, ip, port, num);
	}

	int ListenIPv6(const char* ip, int port, int num) 
	{
		return Listen(AF_INET6, ip, port, num);
	}

	int Listen(int family, const char* ip, int port, int num) 
	{
		if(!ip) return -1;

		int fd = socket(family, SOCK_STREAM, 0);
		if(fd < 0) { 
			return -1;
		}

		if (SetReuseAddr(fd) < 0) {
			close(fd);
			return -1;
		}

		if(Bind(fd, family, ip, port) < 0) {
			close(fd);
			return -1;
		}

		if(listen(fd, num) < 0) {
			close(fd);
			return -1;
		}

		return fd;
	}

	int ConnectIPv4(int sockfd, const char *ip, int port)
	{
		struct sockaddr_in addr = {0};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		if (inet_pton(AF_INET, ip, &addr.sin_addr.s_addr) < 0) {
			return -1;
		}
		return connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
	}

	int ConnectIPv6(int sockfd, const char *ip, int port)
	{
		struct sockaddr_in6 addr6 = {0};
		addr6.sin6_family = AF_INET6;
		addr6.sin6_port = htons(port);
		if (inet_pton(AF_INET6, ip, &addr6.sin6_addr.s6_addr) < 0) {
				return -1;
		}
		return connect(sockfd, (struct sockaddr*)&addr6, sizeof(addr6));
	}

	int Connect(int sockfd, int family, const char *ip, int port)
	{
		if (family == AF_INET6)
			return ConnectIPv6(sockfd, ip, port);

		return ConnectIPv4(sockfd, ip, port);
	}

	int Connect(const char* endpoint, uint16_t port) 
	{
		int ret = 0;
		char buf[2048] = {0};
		struct hostent tmp, *h;
		gethostbyname_r(endpoint, &tmp, buf, sizeof(buf), &h, &ret);
		if (ret != 0) {
			DEBUG("resolve failed. endpoint:%s, port:%d", endpoint, port);
			return -1;
		}

		if (h->h_addrtype != AF_INET) {
			DEBUG("addr type error. endpoint:%s, port:%d", endpoint, port);
			return -1;
		}

		char** head = h->h_addr_list;
		for(; *head; head++)
		{
			int sockfd = socket(AF_INET, SOCK_STREAM, 0);
			struct sockaddr_in sockAddrIn = {0};
			sockAddrIn.sin_family = AF_INET;
			sockAddrIn.sin_port = htons(port);
			sockAddrIn.sin_addr.s_addr = *((unsigned long *)*head);

			while(1) {
				ret = connect(sockfd, (struct sockaddr*)&sockAddrIn, sizeof(sockAddrIn));
				if (ret == 0) {
					DEBUG("connect ok. endpoint:%s, port:%d", endpoint, port);
					return sockfd;
				}

				int errcode = errno;
				if (errcode == EINTR) {
					continue;
				}

				ERROR("connect failed. endpoint:%s, port:%d, code:%d, msg:%s", endpoint, port, errcode, strerror(errcode));
				close(sockfd);
				break;
			}
		}

		ERROR("connect failed. endpoint:%s, port:%d", endpoint, port);
		return -1;
	}

	int ToStrIP(struct sockaddr & addr, string & outIp, int & outPort) 
	{
		char buf[128] = {0};
		if (addr.sa_family == AF_INET6) {
			struct sockaddr_in6 *addr6 = (struct sockaddr_in6*)&addr;
			if (inet_ntop(AF_INET6, (const void*)&addr6->sin6_addr, buf, sizeof(buf)) < 0) {
				return -1;
			}
			outPort = ntohs(addr6->sin6_port);
			outIp = string(buf);
		} else {
			struct sockaddr_in *addr4 = (struct sockaddr_in*)&addr;
			if (inet_ntop(AF_INET, (const void*)&addr4->sin_addr, buf, sizeof(buf)) < 0) {
				return -1;
			}
			outPort = ntohs(addr4->sin_port);
			outIp = string(buf);
		}

		return 0;
	}

	int GetSockOptInt(int sockfd, int level, int optname) 
	{
		int val = 0;
		socklen_t len = sizeof(val);
		if (getsockopt(sockfd, level, optname, &val, &len) < 0) {
			return -1;
		}
		return val;
	}

	int SetSockOptInt(int sockfd, int level, int optname, int val) 
	{
		return setsockopt(sockfd, level, optname, &val,  sizeof(val));
	}

	int GetPeerName(int sockfd, string & outIp, int & outPort)
	{
		struct sockaddr addr = {0};
		socklen_t len = sizeof(addr);
		if (getpeername(sockfd, &addr, &len) < 0) {
			return -1;
		}
		return ToStrIP(addr, outIp, outPort);
	}

	int GetSockName(int sockfd, string& outIp, int& outPort)
	{
		struct sockaddr addr = {0};
		socklen_t len = sizeof(addr);
		if (getsockname(sockfd, &addr, &len) < 0) {
			return -1;
		}
		return ToStrIP(addr, outIp, outPort);
	}

	int GetTCPState(int sockfd)
	{
		return GetSockOptInt(sockfd, IPPROTO_TCP, TCP_INFO); 
	}

	int SetReuseAddr(int sockfd)
	{
		if (SetSockOptInt(sockfd, SOL_SOCKET, SO_REUSEADDR, 1) < 0) {
			return -1;
		}
		return SetSockOptInt(sockfd, SOL_SOCKET, SO_REUSEPORT, 1);
	}

	int SetNoDelay(int sockfd)
	{
		return SetSockOptInt(sockfd, IPPROTO_TCP, TCP_NODELAY, 1);
	}

	int GetRecvLowat(int sockfd)
	{
		return GetSockOptInt(sockfd, SOL_SOCKET, SO_RCVLOWAT);
	}

	int GetSendLowat(int sockfd)
	{
		return GetSockOptInt(sockfd, SOL_SOCKET, SO_SNDLOWAT);
	}

	int SetRecvLowat(int sockfd, int val)
	{
		return setsockopt(sockfd, SOL_SOCKET, SO_RCVLOWAT, &val, sizeof(val));
	}

	int SetSendLowat(int sockfd, int val)
	{
		return setsockopt(sockfd, SOL_SOCKET, SO_SNDLOWAT, &val, sizeof(val));
	}

	int GetRecvBuf(int sockfd)
	{
		return GetSockOptInt(sockfd, SOL_SOCKET, SO_RCVBUF);
	}
	
	int GetSendBuf(int sockfd)
	{
		return GetSockOptInt(sockfd, SOL_SOCKET, SO_SNDBUF);
	}

	int SetRecvBuf(int sockfd, int val)
	{
		return SetSockOptInt(sockfd, SOL_SOCKET, SO_RCVBUF, val);
	}

	int SetSendBuf(int sockfd, int val)
	{
		return SetSockOptInt(sockfd, SOL_SOCKET, SO_SNDBUF, val);
	}

	int GetError(int sockfd)
	{
		return GetSockOptInt(sockfd, SOL_SOCKET, SO_ERROR);
	}

	int SetKeepAlive(int sockfd, int interval)
	{
		if (SetSockOptInt(sockfd, SOL_SOCKET, SO_KEEPALIVE, 1) < 0) {
			return -1;
		}

		if (SetSockOptInt(sockfd, IPPROTO_TCP, TCP_KEEPIDLE, interval) < 0) {
			return -1;
		}

		int val = interval/3;
		if (val == 0) val = 1;
		if (SetSockOptInt(sockfd, IPPROTO_TCP, TCP_KEEPINTVL, val) < 0) {
			return -1;
		}
		
		if (SetSockOptInt(sockfd, IPPROTO_TCP, TCP_KEEPCNT, 3) < 0) {
			return -1;
		}

		return 0;
	}

	string HostName()
	{
		char hostName[256] = {0};
		if (gethostname(hostName, sizeof(hostName)) < 0) {
			return string("");
		}
		return string(hostName);
	}

	int GetHostIPs(const char* hostName, vector<string>& vecIps)
	{
		return Hostent2IPs(gethostbyname(hostName), vecIps);
	}

	int Hostent2IPs(struct hostent* htent, vector<string>& vecIps)
	{
		if (!htent) return -1;

		char buf[128] = {0};
		char** head = htent->h_addr_list;
		for(; *head; head++) {
			inet_ntop(htent->h_addrtype, *head, buf, sizeof(buf));
			vecIps.push_back(buf);
		}
		return 0;
	}

	int LocalHostIps(vector<string>& vecIps)
	{
		char hostName[256] = {0};
		if (gethostname(hostName, sizeof(hostName)) == -1){
			return -1;
		}
		return Hostent2IPs(gethostbyname(hostName), vecIps);
	}

	bool IsIPv4(const std::string& strIp)
	{
		if (strIp.size() < 7 || strIp.size() > 15) 
			return false;

		size_t begin = 0;
		size_t end = 0;
		int cnt = 0;
		int num = 0;
		while((end = strIp.find(".", begin)) != strIp.npos) {
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
}



