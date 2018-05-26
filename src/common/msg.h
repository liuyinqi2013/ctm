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
	typedef enum msg_type
	{
	MSG_BASE = 0,
	MSG_GAME_LOGIN  = 1,
	MSG_GAME_LOGOUT = 2,
	} MSG_TYPE;
	
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

		virtual std::string ToString() const;

		virtual void FromString(const std::string& InBuf);

		virtual const Json::Value& ToJson() const;

		virtual void FromJson(const Json::Value& json);

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
		Json::Value m_root;
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

	template < class T >
	class CTinyMemPool
	{
	public:
		CTinyMemPool(size_t size) : m_size(size), m_sem(size)
		{
			m_array = new T[m_size];
			m_vecMem.push_back(m_array);
			for (int i = 0; i < m_size; ++i) m_listFree.push_back(m_array + i);
		}
		
		~CTinyMemPool()
		{
			for (int i = 0; i < m_vecMem.size(); ++i) delete [] m_vecMem[i];
		}
		
		T* Get()
		{
			m_sem.Wait();
			CLockOwner owner(m_mutex);
			T* pNetPack = m_listFree.front();
			m_listFree.pop_front();

			return pNetPack;
		}
		
		bool Recycle(T* pNetPack)
		{
			CLockOwner owner(m_mutex);
			if (m_array <= pNetPack && pNetPack <=  m_array + m_size - 1)
			{
				m_listFree.push_back(pNetPack);
				m_sem.Post();
				return true;
			}
			
			return false;
		}

		size_t Size() const
		{
			CLockOwner owner(m_mutex);
			return m_size;
		}
		
	private:
		size_t m_size;
		T*     m_array;
		CMutex m_mutex;
		CSem   m_sem;
		std::list<T*> m_listFree;
		std::vector<T*> m_vecMem;
	};

	template < class T >
	class CTinyQueue
	{
	public:
		CTinyQueue() : m_sem(0)
		{
		}
		
		~CTinyQueue()
		{
		}
		
		void Push(const T& t)
		{
			CLockOwner owner(m_mutex);
			m_listT.push_back(t);
			m_sem.Post();
		}
		
		void Pop()
		{
			m_sem.Wait();
			CLockOwner owner(m_mutex);
			m_listT.pop_front();
		}
		
		T& Get()
		{
			CLockOwner owner(m_mutex);
			return m_listT.front();
		}
		
		T GetAndPop()
		{
			m_sem.Wait();
			CLockOwner owner(m_mutex);
			T t = m_listT.front();
			m_listT.pop_front();
			return t;
		}

		size_t Size() const
		{
			CLockOwner owner(m_mutex);
			return m_listT.size();
		}
		
	private:
		CMutex m_mutex;
		CSem   m_sem;
		std::list<T> m_listT;
	};
}


#endif

