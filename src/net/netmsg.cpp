#include "netmsg.h"

namespace ctm
{
	CNetMsg::CNetMsg() :
		CMsg("0", 1, "netmsg"),
		m_iPort(0)	
	{
	}

	CNetMsg::CNetMsg(int port, const std::string& ip, const CSocket& socket, const std::string& buf) :
		CMsg("0", 1, "netmsg"),
		m_iPort(port),
		m_strIp(ip),
		m_sock(socket),
		m_buf(buf)
	{
	}
	
	CNetMsg::~CNetMsg()
	{
	}

	void CNetMsg::TestPrint()
	{
		CMsg::TestPrint();
		DEBUG_LOG("Port = %d", m_iPort);
		DEBUG_LOG("IP  = %s",  m_strIp.c_str());
		DEBUG_LOG("buf  = %s", m_buf.c_str());
	}

	REG_MSG(0, CNetMsg);
}

