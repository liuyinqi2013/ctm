#ifndef CTM_COMMON_MSG_H__
#define CTM_COMMON_MSG_H__
#include <string>
#include <time.h>
#include <map>
#include <vector>
#include "thread/mutex.h"



namespace ctm
{
	class CMsg
	{
	public:
		CMsg() : 
			m_iType(0), 
			m_strName(""),
			m_unixTime(time(NULL))
		{
		}

		CMsg(int type, const std::string& name, time_t t = time(NULL)) : 
			m_iType(type), 
			m_strName(name),
			m_unixTime(t)
		{
		}

		CMsg(const CMsg& other) :
			m_iType(other.m_iType), 
			m_strName(other.m_strName),
			m_unixTime(other.m_unixTime)
		{
		}
			
		virtual ~CMsg() 
		{
		}

		CMsg& operator= (const CMsg& other)
		{
			if (this != &other)
			{
				m_iType = other.m_iType; 
				m_strName = other.m_strName;
				m_unixTime = other.m_unixTime;
			}

			return *this;
		}
		
		virtual bool Serialization(std::string& outBuf); 

		virtual bool DeSerialization(const std::string& InBuf); 

		virtual void TestPrint();

		virtual bool IsValid()
		{
			return true;
		}

	public:
		int m_iType;
		std::string m_strName;
		time_t m_unixTime;
	};

	typedef CMsg* (*CreateMsgFunc)();
	extern std::map<int, CreateMsgFunc> gMapMsg;
	
	
	#define REG_MSG(type, name) \
	inline CMsg* CreateMsg##name() { return new name(); } \
	class CRegMsg##name{ \
	public:\
		CRegMsg##name(int iType) { gMapMsg[type] = CreateMsg##name; }\
	};\
	CRegMsg##name regmsg##name(type)
	
	CMsg* CreateMsg(int type);

	void  DestroyMsg(CMsg* msg);
	
	class CMsgQueue
	{
	
	public:
		CMsgQueue();
		virtual ~CMsgQueue();
		
		virtual void Put(CMsg* pMsg);
		virtual CMsg* Get();
		virtual CMsg* Get(int msgTyep);
		virtual CMsg* Get(const std::string& msgName);

		size_t Size();
	protected:
		int m_iQueueId;
		std::string m_strName;
		CMutex m_mutexLock;
		std::vector<CMsg*> m_msgVec;
	};
}


#endif

