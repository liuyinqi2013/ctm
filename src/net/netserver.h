#ifndef CTM_NET_NETSERVER_H__
#define CTM_NET_NETSERVER_H__
#include <string>
#include <map>
#include "socket.h"
#include "netmsg.h"
#include "thread/thread.h"
#include "thread/mutex.h"

namespace ctm
{
	class ConnInfo : public CMsg
	{
	public:
		ConnInfo(){}
		ConnInfo(const CSocket& sock, const std::string& ip, int port) :
			m_ConnSock(sock),
			m_strConnIp(ip),
			m_iConnPort(port)
		{
		}
			
		~ConnInfo(){}
		
	public:
		CSocket m_ConnSock;
		std::string m_strConnIp;
		int m_iConnPort;
	};

	class CTcpNetServer : public CThread
	{
	public:
		CTcpNetServer(const std::string& ip, int port);
		virtual ~CTcpNetServer();
		bool Init();

		std::string GetServerIp() const
		{
			return m_strIp;
		}

		int GetServerPort() const
		{
			return m_iPort;
		}

		size_t GetConnCount() 
		{
			CLockOwner owner(m_mutexLock);
			return m_mapConns.size();
		}

		CNetMsg* GetMsg()
		{
			return (CNetMsg*)m_RecvQueue.Get();
		}
		
		void PutMsg(CNetMsg* msg)
		{
			m_SendQueue.Put(msg);
		}

		void StartUp();
		
		
	protected:
		
		virtual int Run();

	public:

		void AddClientConn(ConnInfo* conn);

		void DelClientConn(SOCKET_T sock);

		void DelClientConn(ConnInfo* conn);

		int ReadClientConn(ConnInfo* conn);

		int ReadOnePacket(ConnInfo* conn);

		int WriteClientConn(ConnInfo* conn);

		int Readn(ConnInfo* conn, char* buf, int len);
		
	private:
	 	CSocket m_sockFd;
	 	std::string m_strIp;
		int m_iPort;
		std::map<SOCKET_T, ConnInfo*> m_mapConns;
		CMutex m_mutexLock;

		int m_epollFd;

		CMsgQueue m_RecvQueue;
		CMsgQueue m_SendQueue;	
	};

}

#endif

