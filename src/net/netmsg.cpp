#include "netmsg.h"

namespace ctm
{
	CNetMsg::CNetMsg() :
		CMsg(1, "netmsg"),
		m_iPort(0)	
	{
	}
	
	CNetMsg::~CNetMsg()
	{
	}

	void CNetMsg::TestPrint()
	{
		CMsg::TestPrint();
		DEBUG_LOG("Port = %d", m_iPort);
		DEBUG_LOG("IP  = %s", m_strIp.c_str());
		DEBUG_LOG("buf  = %s", m_buf.c_str());
	}

	REG_MSG(0, CNetMsg);
}

