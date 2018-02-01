#ifndef CTM_COMMON_MSG_H__
#define CTM_COMMON_MSG_H__
#include <string>
#include <time.h>
#include <map>
#include <list>
#include "thread/mutex.h"
#include "thread/sem.h"
#include "macro.h"



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

	void  DestroyMsg(CMsg*& msg);
	
	class CMsgQueue
	{
	
	NOCOPY(CMsgQueue)
		
	public:
		CMsgQueue();
		virtual ~CMsgQueue();
		
		virtual void Put(CMsg* pMsg);
		virtual CMsg* Get();
		virtual CMsg* Get(int msgTyep);
		virtual CMsg* Get(const std::string& msgName);

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

