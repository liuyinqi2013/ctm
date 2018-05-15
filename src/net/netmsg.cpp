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
		
		strncpy(ip,   other.ip, sizeof(ip));
		strncpy(ibuf, other.ibuf, sizeof(ibuf));
		strncpy(obuf, other.obuf, sizeof(obuf));
	}

	CNetPackCache::CNetPackCache(int size) :
		m_size(size),
		m_array(new CNetPack[size + 1]),
		m_semFree(size),
		m_semRecv(0),
		m_semSend(0)	
	{
		for (int i = 0; i < m_size; ++i)
			m_vecFree.push_back(m_array + i);
	}
	
	CNetPackCache::~CNetPackCache()
	{
		delete[] m_array;
	}
	
	CNetPack* CNetPackCache::FreePack()
	{
		m_semFree.Wait();
		CLockOwner owner(m_mutexFree);
		CNetPack* pNetPack = m_vecFree.front();
		m_vecFree.pop_front();

		return pNetPack;
		
	}
	
	void CNetPackCache::PutFreeQueue(CNetPack* pNetPack)
	{
		CLockOwner owner(m_mutexFree);
		if (m_array <= pNetPack && pNetPack <=  m_array + m_size - 1)
		{
			m_vecFree.push_back(pNetPack);
			m_semFree.Post();
		}
	}
	
	CNetPack* CNetPackCache::RecvPack()
	{
		m_semRecv.Wait();
		CLockOwner owner(m_mutexRecv);
		CNetPack* pNetPack = m_vecRecv.front();
		m_vecRecv.pop_front();

		return pNetPack;
	}
	
	void CNetPackCache::PutRecvQueue(CNetPack* pNetPack)
	{
		CLockOwner owner(m_mutexRecv);
		if (m_array <= pNetPack && pNetPack <= m_array + m_size - 1)
		{
			m_vecRecv.push_back(pNetPack);
			m_semRecv.Post();
		}
	}
	
	CNetPack* CNetPackCache::SendPack()
	{
		m_semSend.Wait();
		CLockOwner owner(m_mutexSend);
		CNetPack* pNetPack = m_vecSend.front();
		m_vecSend.pop_front();

		return pNetPack;
	}
	
	void CNetPackCache::PutSendQueue(CNetPack* pNetPack)
	{
		CLockOwner owner(m_mutexSend);
		if (m_array <= pNetPack && pNetPack <=  m_array + m_size - 1)
		{
			m_vecSend.push_back(pNetPack);
			m_semSend.Post();
		}
	}
	
	void CNetPackCache::Clear()
	{
		CLockOwner owner0(m_mutexFree);
		CLockOwner owner1(m_mutexRecv);
		CLockOwner owner2(m_mutexSend);
		m_vecFree.clear();
		m_vecRecv.clear();
		m_vecSend.clear();
		for (int i = 0; i < m_size; ++i)
			m_vecFree.push_back(m_array + i);
	}


	void CNetPackCache::AddContext(int sock, CNetPack* pNetPack)
	{
		m_mapContext[sock] = pNetPack;
	}

	void CNetPackCache::DelContext(int sock)
	{
		std::map<int, CNetPack*>::iterator it = m_mapContext.find(sock);
		if (it != m_mapContext.end())
		{
			m_mapContext.erase(it);
		}
	}

	CNetPack* CNetPackCache::GetContext(int sock)
	{
		std::map<int, CNetPack*>::iterator it = m_mapContext.find(sock);
		if (it != m_mapContext.end())
		{
			return it->second;
		}
		
		return NULL;
	}
	

	CNetPackQueue CNetPackQueue::RecvQueue;
	CNetPackQueue CNetPackQueue::SendQueue;

	CNetPackQueue::CNetPackQueue(int size) :
		m_size(size),
		m_semFree(size),
		m_semUsed(0)
	{
		m_array = new CNetPack[m_size + 1];
		for (int i = 0; i < m_size; ++i)
			m_freePack.push_back(m_array + i);
	}
	
	CNetPackQueue::~CNetPackQueue()
	{
		delete[] m_array;
	}

	CNetPack* CNetPackQueue::GetFreePack()
	{
		m_semFree.Wait();
		CLockOwner owner(m_mutex);

		if (m_freePack.size() == 0)
			return NULL;

		CNetPack* pNetPack = *m_freePack.begin();
		m_freePack.erase(m_freePack.begin());
		
		return pNetPack;
	}
	
	CNetPack* CNetPackQueue::GetUsedPack()
	{
		m_semUsed.Wait();
		CLockOwner owner(m_mutex);
		if (m_usedPack.size() == 0)
			return NULL;
		
		CNetPack* pNetPack = *m_usedPack.begin();
		m_usedPack.erase(m_usedPack.begin());

		return pNetPack;
	}

	void CNetPackQueue::SaveFreePack(CNetPack* pNetPack)
	{
		CLockOwner owner(m_mutex);
		if (m_array <= pNetPack && pNetPack <=  m_array + m_size - 1)
		{
			m_freePack.push_back(pNetPack);
			m_semFree.Post();
		}
		
	}
	
	void CNetPackQueue::SaveUsedPack(CNetPack* pNetPack)
	{
		CLockOwner owner(m_mutex);
		if (m_array <= pNetPack && pNetPack <=  m_array + m_size - 1)
		{
			m_usedPack.push_back(pNetPack);
			m_semFree.Post();
		}
	}

	void CNetPackQueue::Clear()
	{
		CLockOwner owner(m_mutex);
		
		m_freePack.clear();
		m_usedPack.clear();
		for (int i = 0; i < m_size; ++i)
			m_freePack.push_back(m_array + i);
	}

}

