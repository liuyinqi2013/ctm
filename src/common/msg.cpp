#include "msg.h"
#include "macro.h"
#include "lock.h"

namespace ctm
{
	std::map<int, CreateMessageFunction> global_message_Map;

	CMsg* CreateMsg(int type)
	{
		return global_message_Map[type]();
	}

	void  DestroyMsg(CMsg*& msg)
	{
		if (msg)
		{
			delete msg;
			msg = NULL;
		}
	}
	
	bool CMsg::Serialization(std::string& outBuf)
	{
		Json::Value root;
		root["id"] = m_strId;
		root["type"] = m_iType;
		root["name"] = m_strName;
		root["unixTime"] = m_unixTime;
		outBuf = root.toStyledString();
		
		return true;
	}
	
	bool CMsg::DeSerialization(const std::string& InBuf)
	{
		Json::Value root;
		Json::Reader reader;
		if (!reader.parse(InBuf, root))
		{
			return false;
		}

		m_strId = root["id"].asString();
		m_iType = root["type"].asInt();
		m_strName = root["name"].asString();
		m_unixTime = root["unixTime"].asUInt();
		
		return true;
		
	}

	std::string CMsg::ToString()
	{
		return ToJson().toStyledString();
	}

	void CMsg::FromString(const std::string& InBuf) 
	{
		Json::Value root;
		Json::Reader reader;
		if (!reader.parse(InBuf, root))
		{
			return ;
		}
		
		FromJson(root);
	}

	const Json::Value& CMsg::ToJson()
	{
		m_root["id"] = m_strId;
		m_root["type"] = m_iType;
		m_root["name"] = m_strName;
		m_root["unixTime"] = m_unixTime;

		return m_root;
	}

	void CMsg::FromJson(const Json::Value& json) 
	{
		m_strId = json["id"].asString();
		m_iType = json["type"].asInt();
		m_strName = json["name"].asString();
		m_unixTime = json["unixTime"].asUInt();
	}

	void CMsg::TestPrint()
	{
		DEBUG_LOG("MSG id   = %s", m_strId.c_str());
		DEBUG_LOG("MSG type = %d", m_iType);
		DEBUG_LOG("MSG name = %s", m_strName.c_str());
		DEBUG_LOG("MSG time = %d", m_unixTime);
	}

	REG_MSG(0, CMsg);

	CMsgQueue::CMsgQueue() :
		m_iQueueId(0),
		m_strName(""),
		m_maxSize(1024),
		m_sem(0)
		
	{
	}
	
	CMsgQueue::~CMsgQueue()
	{
	}
		
 	bool CMsgQueue::Put(CMsg* pMsg)
 	{
 		if (!pMsg) return false;
		
 		CLockOwner owner(m_mutexLock);
		
		if (m_msgVec.size() >= m_maxSize)
		{
			ERROR_LOG("message queue already full max size : %d", m_maxSize);
			return false;
		}

		m_msgVec.push_back(pMsg);

		m_sem.Post();

		return true;
 	}

	CMsg* CMsgQueue::Get()
	{	
		CMsg* pMSG = NULL;

		m_sem.Wait();
		
		CLockOwner owner(m_mutexLock);
		
		if (m_msgVec.begin() != m_msgVec.end())
		{
			pMSG = *m_msgVec.begin();
			m_msgVec.erase(m_msgVec.begin());
		}
			
		return pMSG;
	}

	/*
	CMsg* CMsgQueue::Get(int msgTyep)
	{
		CMsg* pMSG = NULL;

		m_sem.Wait();
		
		CLockOwner owner(m_mutexLock);
		
		std::list<CMsg*>::iterator it = m_msgVec.begin();
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
		std::list<CMsg*>::iterator it = m_msgVec.begin();
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

	*/

	size_t CMsgQueue::Size()
	{
		CLockOwner owner(m_mutexLock);
		return m_msgVec.size();
	}

}


