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
	enum msg_type
	{
		MSG_BASE = 0,
		MSG_GAME = 1,
		MSG_GAME_LOGIN_C2S = 2,
		MSG_GAME_LOGIN_S2C = 3,
		MSG_GAME_LOGOUT_C2S = 4,
		MSG_GAME_LOGOUT_S2C = 5,
		MSG_GAME_JOIN_GAME_C2S = 6,
		MSG_GAME_JOIN_GAME_S2C = 7,
		MSG_GAME_GAME_BEGIN_S2C = 8,
		MSG_GAME_CALL_DIZHU_C2S = 9,
		MSG_GAME_CALL_DIZHU_S2C = 10,
		MSG_GAME_OPT_S2C = 11,
		MSG_GAME_OUT_CARD_C2S = 12,
		MSG_GAME_OUT_CARD_S2C = 14,
		MSG_GAME_GAME_OVER_S2C = 15,
		MSG_GAME_GAME_OVER_OPT = 16,
		MSG_GAME_GAME_INFO_S2C = 17,

		MSG_GAME_PLAYER_INFO = 100,
		MSG_GAME_PLAYER_ARRAY = 101,

		MSG_SYSTEM_NET = 1000,
	
	};
	
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

		CMsg& operator= (const CMsg& other);
		
		virtual bool Serialization(std::string& outBuf); 

		virtual bool DeSerialization(const std::string& InBuf);

		virtual std::string ToString();

		virtual void FromString(const std::string& InBuf);

		virtual const Json::Value& ToJson();

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

	typedef CMsg* (*CreateMessageFunction)();
	extern std::map<int, CreateMessageFunction> global_message_Map;
	
	#define REG_MSG(msg_type, class_name)\
	static inline CMsg* function_##class_name() { return new class_name(); }\
	class reg_msg_##class_name\
	{\
	public:\
		reg_msg_##class_name() { global_message_Map[msg_type] = function_##class_name; }\
	};\
	static reg_msg_##class_name tmp_##class_name()
	
	CMsg* CreateMsg(int type);

	void  DestroyMsg(CMsg*& msg);
	
	class CMsgQueue
	{
	
	DISABLE_COPY_ASSIGN(CMsgQueue)
		
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
			for (size_t i = 0; i < m_size; ++i) m_listFree.push_back(m_array + i);
		}
		
		~CTinyMemPool()
		{
			for (size_t i = 0; i < m_vecMem.size(); ++i) delete [] m_vecMem[i];
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

