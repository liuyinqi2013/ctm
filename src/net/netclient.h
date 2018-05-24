#ifndef CTM_NET_NETCLIENT_H__
#define CTM_NET_NETCLIENT_H__
#include "socket.h"
#include "netmsg.h"
#include "thread/thread.h"

#include <sys/select.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <string>
namespace ctm
{
	class CNetTcpClient : public CThread
	{
	public:
		CNetTcpClient() 
		{
		}
		
		CNetTcpClient(const std::string& ip, int port) : m_strServerIp(ip), m_iPort(port)
		{
		}
		
		virtual ~CNetTcpClient()
		{
		}

		bool Init();

		std::string GetNetPack()
		{
			return m_recvQueue.GetAndPop();
		}

		int SendNetPack(const std::string& buf)
		{
			return m_Socket.Send(m_Context.Pack(buf));
		}

		void SetEndFlag(const std::string& endflag)
		{
			m_Context.SetSep(endflag);
		}

	protected:
		
		virtual int Run();
		
	private:
		int m_iPort;
		std::string m_strServerIp;
		CSocket m_Socket;
		
		CTinyQueue<std::string>  m_recvQueue;
		CNetContext m_Context;

		fd_set m_readFds;
	};
}

#endif

