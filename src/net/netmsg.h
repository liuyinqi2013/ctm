#ifndef CTM_NET_NETMSG_H__
#define CTM_NET_NETMSG_H__

#include "socket.h"
#include "common/msg.h"

namespace ctm
{
	class CNetMsg : public CMsg
	{
	public:
		CNetMsg();
		CNetMsg(int port, const std::string& ip, const CSocket& socket, const std::string& buf);
		virtual ~CNetMsg();
		
		void TestPrint();
		
	public:
		int m_iPort;
		std::string m_strIp;
		CSocket m_sock;
		std::string m_buf;
	};
}

#endif

