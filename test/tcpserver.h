#ifndef CTM_TEST_TCPSERVER_H__
#define CTM_TEST_TCPSERVER_H__

#include "net/socket.h"
#include "net/select.h"
#include <map>

using namespace ctm;

class TcpServer
{
public:
	TcpServer(const std::string& ip, int port);
	virtual ~TcpServer();
	void Run();
private:
	CSocket m_tcpSock;
	std::string m_ip;
	int m_port;
	std::map<SOCKET_T, CSocket> m_sockClients;
};

#endif

