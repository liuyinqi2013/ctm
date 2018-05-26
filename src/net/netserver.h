#ifndef CTM_NET_NETSERVER_H__
#define CTM_NET_NETSERVER_H__
#include <string>
#include <map>
#include <vector>
#include "socket.h"
#include "netmsg.h"
#include "thread/thread.h"
#include "thread/mutex.h"
#include "common/com.h"

namespace ctm
{
	class ClientConnect
	{
	public:
		ClientConnect(){}
		ClientConnect(const CSocket& sock, const std::string& ip, int port) :
			m_ConnSock(sock),
			m_strConnIp(ip),
			m_iConnPort(port),
			m_iStatus(0)
		{
		}
			
		~ClientConnect(){}

		std::string ToString() const
		{
			return I2S(m_ConnSock.GetSock()) + " " + m_strConnIp + ":" + I2S(m_iConnPort);
		}
		
	public:
		CSocket m_ConnSock;
		std::string m_strConnIp;
		int m_iConnPort;
		int m_iStatus;
	};

	class CTcpNetServer : public CThread
	{
		
	protected:
		class CNetSendThread : public CThread
		{
		public:
			CNetSendThread(){}
			virtual ~CNetSendThread(){}
			void SetNetServer(CTcpNetServer* pServer)
			{
				m_tcpNetServer = pServer;
			}
			
		protected:
			virtual int Run();
			
		private:
			CTcpNetServer* m_tcpNetServer;
		};

		class CNetRecvThread : public CThread
		{
		public:
			CNetRecvThread(){}
			virtual ~CNetRecvThread(){}
			void SetNetServer(CTcpNetServer* pServer)
			{
				m_tcpNetServer = pServer;
			}
			
		protected:
			virtual int Run();
			
		private:
			CTcpNetServer* m_tcpNetServer;
		};
		
	public:
		CTcpNetServer(const std::string& ip, int port);
		virtual ~CTcpNetServer();
		bool Init();

		void ShutDown();

		const std::string& ServerIp() const
		{
			return m_strIp;
		}

		int ServerPort() const
		{
			return m_iPort;
		}

		size_t ClientCount() 
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

		int SendThreadNum() const
		{
			return m_sendThreadNum;
		}

		void SetSendThreadNum(int n)
		{
			m_sendThreadNum = n;
		}

		int RecvThreadNum() const
		{
			return m_recvThreadNum;
		}

		void SetRecvThreadNum(int n)
		{
			m_recvThreadNum = n;
		}

		void Recycle(CNetPack* p)
		{
			m_netPackPool.Recycle(p);
		}
		
	protected:
		
		virtual int Run();

	private:

		ClientConnect* GetClientConnect(SOCKET_T sock);
		
		bool AddClientConnect(ClientConnect* conn);

		bool DelClientConnect(SOCKET_T sock);

		bool DelClientConnect(ClientConnect* conn);

		int ReadClientConnect(ClientConnect* conn);

		int ReadOnePacket(ClientConnect* conn);

		int Readn(ClientConnect* conn, char* buf, int len);

		void ClientRecv();
		
		void ClientSend();
	
	private:
	 	CSocket m_sockFd;
	 	std::string m_strIp;
		int m_iPort;
		std::map<SOCKET_T, ClientConnect*> m_mapConns;
		CMutex m_mutexLock;

		int m_epollFd;

		int m_sendThreadNum;
		int m_recvThreadNum;
		CNetSendThread* m_pSendThread;
		CNetRecvThread* m_pRecvThread;
		std::string m_endFlag;

		CTinyMemPool<CNetPack> m_netPackPool;
		CTinyQueue<CNetPack*>  m_recvQueue;
		CTinyQueue<CNetPack*>  m_sendQueue;
		CTinyQueue<ClientConnect*> m_readableConnQueue;
		CNetContext m_Context;

		friend class CNetSendThread;
		friend class CNetRecvThread;
	};

}

#endif

