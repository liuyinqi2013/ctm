#ifndef CTM_NET_NETMSG_H__
#define CTM_NET_NETMSG_H__

#include "socket.h"
#include "common/msg.h"
#include "thread/mutex.h"
#include "thread/sem.h"

#include <vector>
#include <list>
#include <map>
#include <stdlib.h>

#define BUF_MAX_SIZE 16 * 1024
namespace ctm
{
	class CNetMsg : public CMsg
	{
	public:
		CNetMsg();
		CNetMsg(int port, const std::string& ip, const CSocket& socket, const std::string& buf);
		virtual ~CNetMsg();
		
		void TestPrint();
		
	public:
		int m_iPort;
		std::string m_strIp;
		CSocket m_sock;
		std::string m_buf;
	};

	typedef struct CNetPack
	{
		CNetPack();
		CNetPack(const CNetPack& other);
		void TestPrint();
		void Copy(const CNetPack& other);
		
		int  sock;
		char ip[16];
		int  port;
		int  ilen;
		char ibuf[BUF_MAX_SIZE];
		int  olen;
		char obuf[BUF_MAX_SIZE];
		char temp[1024];
	} CNetPack;

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
			CNetPack* pNetPack = m_listFree.front();
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
		
		void Push(T* p)
		{
			CLockOwner owner(m_mutex);
			m_listT.push_back(p);
			m_sem.Post();
		}
		
		void Pop()
		{
			m_sem.Wait();
			CLockOwner owner(m_mutex);
			m_listT.pop_front();
		}
		
		T* Get()
		{
			CLockOwner owner(m_mutex);
			return m_listT.front();
		}
		
		T* GetAndPop()
		{
			m_sem.Wait();
			CLockOwner owner(m_mutex);
			CNetPack* p = m_listT.front();
			m_listT.pop_front();
			return p;
		}
	private:
		CMutex m_mutex;
		CSem   m_sem;
		std::list<T*> m_listT;
	};

	class CNetContext
	{
	public:
		CNetContext(const std::string& sep = "\r\n") : m_sep(sep) 
		{
		}
		
		~CNetContext()
		{
		}
 
		void SetSep(const std::string& sep) 
		{
			CLockOwner owner(m_mutex);
			m_sep = sep;
		}

		void Clear()
		{
			CLockOwner owner(m_mutex);
			m_mapContext.clear();
		}
		
		void GetCompletePack(SOCKET_T sock,  const std::string& buf, std::vector<std::string>& vecOut);
		void Remove(SOCKET_T sock);
	private:
		std::string m_sep; //分隔符
		CMutex m_mutex;
		std::map<SOCKET_T, std::string> m_mapContext; //上下文
	};
}

#endif

