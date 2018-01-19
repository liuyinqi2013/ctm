#ifndef CTM_COMMON_MSG_H__
#define CTM_COMMON_MSG_H__
#include <string>
#include <time.h>
#include <map>


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
			
		virtual ~CMsg() 
		{
		}

		virtual bool Serialization(std::string& outBuf); 

		virtual bool DeSerialization(const std::string& InBuf); 

		virtual void TestPrint();
		
	public:
		int m_iType;
		std::string m_strName;
		time_t m_unixTime;
	};

	typedef CMsg* (*CreateMsgFunc)();
	extern std::map<int, CreateMsgFunc> gMapMsg;
	
	
	#define REG_MSG(type, name) \
	inline CMsg* CreateMsg_name() { return new name(); } \
	class CRegMsg_name{ \
	public:\
		CRegMsg_name(int iType) { gMapMsg[type] = CreateMsg_name; }\
	};\
	CRegMsg_name regmsg_name(type)
	
	CMsg* CreateMsg(int type);
	
	class CMsgQueue
	{
	public:
		CMsgQueue() {}
		virtual ~CMsgQueue() {}
		
		//virtual void Put(const shared_ptr<CMsg*>& pMsg) = 0;
		//virtual shared_ptr<CMsg*> Get() = 0;
		//virtual shared_ptr<CMsg*> Get(int msgTyep) = 0;
				
	};
}


#endif

