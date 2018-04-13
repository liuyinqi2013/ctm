#ifndef CTM_COMMON_MSG_H__
#define CTM_COMMON_MSG_H__
#include <string>
#include <time.h>
#include <map>
#include <list>
#include "thread/mutex.h"
#include "thread/sem.h"
#include "macro.h"
#include "json/json.h"
#include "json/json-forwards.h"

namespace ctm
{
	class CMsg
	{
	public:
		CMsg() : 
			m_strId("0"),
			m_iType(0), 
			m_strName(""),
			m_unixTime(time(NULL))
		{
		}

		CMsg(const std::string& id, int type, const std::string& name, time_t t = time(NULL)) : 
			m_strId(id),
			m_iType(type), 
			m_strName(name),
			m_unixTime(t)
		{
		}

		CMsg(const CMsg& other) : 
			m_strId(other.m_strId),
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
				m_strId = other.m_strId;
				m_iType = other.m_iType; 
				m_strName = other.m_strName;
				m_unixTime = other.m_unixTime;
			}

			return *this;
		}
		
		virtual bool Serialization(std::string& outBuf); 

		virtual bool DeSerialization(const std::string& InBuf); 

		virtual void PutToJson(Json::Value& root);

		virtual void GetFromJson(Json::Value& root);

		virtual void TestPrint();

		virtual bool IsValid()
		{
			return true;
		}

		std::string Id() const
		{
			return m_strId;
		}

		int Type() const
		{
			return m_iType;
		}

		std::string Name() const
		{
			return m_strName;
		}

		void SetId(const std::string& id)
		{
			m_strId = id;
		}

		void SetType(const int type)
		{
			m_iType = type;
		}

		void SetName(const std::string& name)
		{
			m_strName = name;
		}
		
	public:
		std::string m_strId;
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

	void  DestroyMsg(CMsg*& msg);
	
	class CMsgQueue
	{
	
	NOCOPY(CMsgQueue)
		
	public:
		CMsgQueue();
		virtual ~CMsgQueue();
		
		virtual bool Put(CMsg* pMsg);
		virtual CMsg* Get();
		
		int Id() const
		{
			return m_iQueueId;
		}

		void SetId(int id)
		{
			m_iQueueId = id;
		}

		std::string Name() const
		{
			return m_strName;
		}

		void SetName(const std::string& name)
		{
			m_strName = name;
		}
		
		size_t MaxSize() const
		{
			return m_maxSize;
		}

		void SetMaxSize(size_t maxSize)
		{
			m_maxSize = maxSize;
		}

		size_t Size();

		bool IsFull()
		{
			return (Size() >= m_maxSize);
		}
		
	protected:
		int m_iQueueId;
		std::string m_strName;
		CMutex m_mutexLock;
		std::list<CMsg*> m_msgVec;
		size_t m_maxSize;
		CSem   m_sem;
	};
}


#endif

