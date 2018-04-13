#include "netclient.h"

namespace ctm
{
	bool CNetTcpClient::Connect()		
	{
		if (!IsValidIp(m_strServerIp))
		{
			ERROR_LOG("server ip invalid : %s", m_strServerIp.c_str());
			return false;
		}

		return m_Socket.Connect(m_strServerIp, m_iPort);
	}

	int CNetTcpClient::Send(const char* buf, size_t len)
	{
		return m_Socket.Send(buf, len);
	}

	int CNetTcpClient::Recv(char* buf, size_t len)
	{
		return m_Socket.Recv(buf, len);
	}
}
