#include "netmsg.h"
#include "ipc/semaphore.h"
#include "common/string_tools.h"

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

	CNetPack::CNetPack() :
		sock(-1),
		port(-1),
		ilen(0),
		olen(0)
	{
		memset(ip, 0, sizeof(ip));
		memset(ibuf, 0, sizeof(ibuf));
		memset(obuf, 0, sizeof(obuf));
		memset(temp, 0, sizeof(temp));
	}
	
	CNetPack::CNetPack(const CNetPack& other)
	{
		Copy(other);
	}

	void CNetPack::TestPrint()
	{
		DEBUG_LOG("sock  = %d",  sock);
		DEBUG_LOG("ip  = %s",  ip);
		DEBUG_LOG("port = %d", port);
		DEBUG_LOG("ilen  = %d", ilen);
		DEBUG_LOG("ibuf  = %s", ibuf);
		DEBUG_LOG("olen  = %d", olen);
		DEBUG_LOG("obuf  = %s", obuf);
	}

	void CNetPack::Copy(const CNetPack& other)
	{
		sock = other.sock;
		port = other.port;
		ilen = other.ilen;
		olen = other.olen;
		
		memcpy(ip,   other.ip, sizeof(ip));
		memcpy(ibuf, other.ibuf, sizeof(ibuf));
		memcpy(obuf, other.obuf, sizeof(obuf));
	}

	void CNetContext::GetCompletePack(SOCKET_T sock, const std::string& buf, std::vector<std::string>& vecOut)
	{
		CLockOwner owner(m_mutex);
		std::string tmp = buf;
		std::map<int, std::string>::iterator it = m_mapContext.find(sock);
		if (it != m_mapContext.end())
		{
			tmp = it->second + buf;
			m_mapContext.erase(it);
		}

		CutString(tmp, vecOut, m_sep, false);
		if (!EndsWith(tmp, m_sep) && vecOut.size() > 0)
		{
			m_mapContext[sock] = vecOut.back();
			vecOut.pop_back();
		}
	}
	
	void CNetContext::Remove(SOCKET_T sock)
	{
		CLockOwner owner(m_mutex);
		std::map<int, std::string>::iterator it = m_mapContext.find(sock);
		if (it != m_mapContext.end())
		{
			m_mapContext.erase(it);
		}
	}
}

