#ifndef CTM_NET_NETMSG_H__
#define CTM_NET_NETMSG_H__

#include "socket.h"
#include "common/msg.h"
#include "thread/mutex.h"
#include "thread/sem.h"

#include <vector>
#include <list>
#include <map>

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

	class CNetPackCache
	{
	public:
		CNetPackCache(int size = 128);
		~CNetPackCache();

		CNetPack* FreePack();
		void PutFreeQueue(CNetPack* pNetPack);

		CNetPack* RecvPack();
		void PutRecvQueue(CNetPack* pNetPack);

		CNetPack* SendPack();
		void PutSendQueue(CNetPack* pNetPack);
		
		void Clear();

		void AddContext(int sock, CNetPack* pNetPack); //增加上下文

		void DelContext(int sock);  //删除上下文

		CNetPack* GetContext(int sock); //获取上下文

	private:
		int m_size;
		CNetPack* m_array;
		CSem   m_semFree;
		CSem   m_semRecv;
		CSem   m_semSend;
		CMutex    m_mutexFree;
		CMutex    m_mutexRecv;
		CMutex    m_mutexSend;
		std::list<CNetPack*> m_vecFree;
		std::list<CNetPack*> m_vecRecv;
		std::list<CNetPack*> m_vecSend;
		std::map<int, CNetPack*> m_mapContext; //上下文
	};

	class CNetPackQueue
	{
	public:
		CNetPackQueue(int size = 64);
		~CNetPackQueue();
		
		CNetPack* GetFreePack();
		CNetPack* GetUsedPack();

		void SaveFreePack(CNetPack* pNetPack);
		void SaveUsedPack(CNetPack* pNetPack);
		
		void Clear();
	private:
		int m_size;
		CNetPack* m_array;
		CMutex    m_mutex;
		CSem   m_semFree;
		CSem   m_semUsed;
		std::vector<CNetPack*> m_freePack;
		std::vector<CNetPack*> m_usedPack;
	public:
		static CNetPackQueue RecvQueue;
		static CNetPackQueue SendQueue;
	};
	
}

#endif

