#ifndef CTM_NET_NETCLIENT_H__
#define CTM_NET_NETCLIENT_H__
#include "socket.h"
#include "netmsg.h"
#include <string>
namespace ctm
{
	class CNetTcpClient
	{
	public:
		CNetTcpClient() 
		{
		}
		
		CNetTcpClient(std::string& ip, int port) : m_strServerIp(ip), m_iPort(port)
		{
		}
		
		~CNetTcpClient()
		{
		}
		
		bool Connect();
		
		bool Connect(std::string& ip, int port)
		{
			m_strServerIp = ip;
			m_iPort = port;
			return Connect();
		}

		int Send(const char* buf, size_t len);

		int Recv(char* buf, size_t len);
		
	private:
		int m_iPort;
		std::string m_strServerIp;
		CSocket m_Socket;
	};
}

#endif

