#ifndef CTM_NET_SOCKET_H__
#define CTM_NET_SOCKET_H__

#include <vector>
#include <string>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

namespace ctm {
	using namespace std;

	int Accept(int sockfd, struct sockaddr_in *addr);

	int BindIPv4(int sockfd, const char *ip, int port);
	int BindIPv6(int sockfd, const char *ip, int port);
	int Bind(int sockfd, int family, const char *ip, int port);

	int ListenIPv4(const char* ip, int port, int num = 5);
	int ListenIPv6(const char* ip, int port, int num = 5);
	int Listen(int family, const char* ip, int port, int num = 5);

	int ConnectIPv4(int sockfd, const char *ip, int port);
	int ConnectIPv6(int sockfd, const char *ip, int port);
	int Connect(int sockfd, int family, const char *ip, int port);
	int Connect(const char* endpoint, uint16_t port);

	int ToStrIP(struct sockaddr & addr, string & outIp, int & outPort);

	int GetSockOptInt(int sockfd, int level, int optname);
	int SetSockOptInt(int sockfd, int level, int optname, int val);

	int GetPeerName(int sockfd, string& outIp, int& outPort);
	int GetSockName(int sockfd, string& outIp, int& outPort);
	int GetTCPState(int sockfd);

	int SetReuseAddr(int sockfd);
	int SetNoDelay(int sockfd);

	int GetRecvLowat(int sockfd);
	int GetSendLowat(int sockfd);
	int SetRecvLowat(int sockfd, int val);
	int SetSendLowat(int sockfd, int val);

	int GetRecvBuf(int sockfd);
	int GetSendBuf(int sockfd);
	int SetRecvBuf(int sockfd, int val);
	int SetSendBuf(int sockfd, int val);

	int GetError(int sockfd);
	int SetKeepAlive(int sockfd, int interval);

	bool IsIPv4(const string& strIp);
	bool IsIPv6(const string& strIp);
	string HostName();
	
	int Hostent2IPs(struct hostent* htent, vector<string>& vecIps);
	int GetHostIPs(const char* hostName, vector<string>& vecIps);
	int LocalHostIps(vector<string>& vecIps);
}

#endif
