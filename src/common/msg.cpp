#include "msg.h"
#include "macro.h"

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

}


