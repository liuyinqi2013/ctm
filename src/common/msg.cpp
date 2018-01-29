#include "msg.h"
#include "macro.h"
#include "lock.h"

namespace ctm
{
	std::map<int, CreateMsgFunc> gMapMsg;

	CMsg* CreateMsg(int type)
	{
		return gMapMsg[type]();
	}
	
	bool CMsg::Serialization(std::string& outBuf)
	{
		outBuf.clear();
		outBuf.append((const char*)&m_iType, sizeof(m_iType));
		int size = m_strName.size();
		outBuf.append((const char*)&size, sizeof(size));
		outBuf.append(m_strName);
		outBuf.append((const char*)&m_unixTime, sizeof(m_unixTime));
		return true;
	}
	
	bool CMsg::DeSerialization(const std::string& InBuf)
	{
		int pos = 0;
		int len = InBuf.copy((char*)&m_iType, sizeof(m_iType), pos);
		if (len != sizeof(m_iType))
		{
			return false;
		}
		pos += len;
		int size = 0;
		len = InBuf.copy((char*)&size, sizeof(size), pos);
		if (len != sizeof(size))
		{
			return false;
		}
		pos += len;
		
		char* buf =  new char[size + 1];
		if (!buf) 
		{
			return false;
		}

		len = InBuf.copy((char*)buf, size, pos);
		if (len != size)
		{
			delete[] buf;
			return false;
		}
		pos += len;
		buf[size] = '\0';
		m_strName = buf;
		delete[] buf;

		len = InBuf.copy((char*)&m_unixTime, sizeof(m_unixTime), pos);
		if (len != sizeof(m_unixTime))
		{
			return false;
		}

		return true;
		
	}

	void CMsg::TestPrint()
	{
		DEBUG_LOG("MSG type = %d", m_iType);
		DEBUG_LOG("MSG name = %s", m_strName.c_str());
		DEBUG_LOG("MSG time = %d", m_unixTime);
	}

	REG_MSG(0, CMsg);

	CMsgQueue::CMsgQueue() :
		m_iQueueId(0),
		m_strName("")	
		
	{
	}
	
	CMsgQueue::~CMsgQueue()
	{
	}
		
 	void CMsgQueue::Put(CMsg* pMsg)
 	{
 		if (!pMsg) return;
		
 		CLockOwner owner(m_mutexLock);
		m_msgVec.push_back(pMsg);
 	}

	CMsg* CMsgQueue::Get()
	{	
		CMsg* pMSG = NULL;
		
		CLockOwner owner(m_mutexLock);
		if (m_msgVec.begin() != m_msgVec.end())
		{
			pMSG = *m_msgVec.begin();
			m_msgVec.erase(m_msgVec.begin());
		}
			
		return pMSG;
	}
	
	CMsg* CMsgQueue::Get(int msgTyep)
	{
		CMsg* pMSG = NULL;
		
		CLockOwner owner(m_mutexLock);
		std::vector<CMsg*>::iterator it = m_msgVec.begin();
		for (; it != m_msgVec.end(); it++)
		{
			if (*it && (*it)->m_iType == msgTyep) 
			{
				pMSG = *it;
				m_msgVec.erase(it);
				break;
			}
		}
			
		return pMSG;
	}

	CMsg* CMsgQueue::Get(const std::string& msgName)
	{
		CMsg* pMSG = NULL;
		
		CLockOwner owner(m_mutexLock);
		std::vector<CMsg*>::iterator it = m_msgVec.begin();
		for (; it != m_msgVec.end(); it++)
		{
			if (*it && (*it)->m_strName== msgName) 
			{
				pMSG = *it;
				m_msgVec.erase(it);
				break;
			}
		}
			
		return pMSG;
	}

	size_t CMsgQueue::Size()
	{
		CLockOwner owner(m_mutexLock);
		return m_msgVec.size();
	}

}


