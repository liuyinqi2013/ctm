#ifndef CTM_NET_NETSERVER_H__
#define CTM_NET_NETSERVER_H__
#include <string>
#include <map>
#include <vector>
#include "socket.h"
#include "netmsg.h"
#include "thread/thread.h"
#include "thread/mutex.h"

namespace ctm
{
	class CliConn : public CMsg
	{
	public:
		CliConn(){}
		CliConn(const CSocket& sock, const std::string& ip, int port) :
			m_ConnSock(sock),
			m_strConnIp(ip),
			m_iConnPort(port)
		{
		}
			
		~CliConn(){}
		
	public:
		CSocket m_ConnSock;
		std::string m_strConnIp;
		int m_iConnPort;
	};

	class CTcpNetServer;

	class CSendThread : public CThread
	{
	public:
		CSendThread()
		{
		}
		
		virtual ~CSendThread()
		{
		}

		void SetNetServer(CTcpNetServer* pServer)
		{
			m_tcpNetServer = pServer;
		}
		
	protected:
		virtual int Run();
	private:
		CTcpNetServer* m_tcpNetServer;
	};

	class CTcpNetServer : public CThread
	{
		friend class CSendThread;
		
	public:
		CTcpNetServer(const std::string& ip, int port);
		virtual ~CTcpNetServer();
		bool Init();

		void ShutDown();

		const std::string& GetServerIp() const
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

		CNetPack* GetNetPack()
		{
			return m_recvQueue.GetAndPop();
		}

		void SendNetPack(CNetPack* pNetPack)
		{
			return m_sendQueue.Push(pNetPack);
		}

		void StartUp();

		const std::string& GetEndFlag() const
		{
			return m_endFlag;
		}

		void SetEndFlag(const std::string& endflag)
		{
			m_endFlag = endflag;
			m_Context.SetSep(endflag);
		}
		
	protected:
		
		virtual int Run();

	private:

		CliConn* GetCliConn(SOCKET_T sock);
		
		bool AddCliConn(CliConn* conn);

		bool DelCliConn(SOCKET_T sock);

		bool DelCliConn(CliConn* conn);

		int ReadCliConn(CliConn* conn);

		int ReadOnePacket(CliConn* conn);

		int WriteCliConn(CliConn* conn);

		int Readn(CliConn* conn, char* buf, int len);

		void AddContext(SOCKET_T sock, CNetPack* pNetPack); //增加上下文

		void DelContext(SOCKET_T sock);  //删除上下文

		CNetPack* GetContext(SOCKET_T sock); //获取上下文
		
	private:
	 	CSocket m_sockFd;
	 	std::string m_strIp;
		int m_iPort;
		std::map<SOCKET_T, CliConn*> m_mapConns;
		CMutex m_mutexLock;

		int m_epollFd;

		CSendThread m_sendThread;
		std::string m_endFlag;

		CTinyMemPool<CNetPack> m_netPackPool;
		CTinyQueue<CNetPack> m_recvQueue;
		CTinyQueue<CNetPack> m_sendQueue;
		std::map<SOCKET_T, CNetPack*> m_mapContext; //上下文

		CNetContext m_Context;
	};

}

#endif

