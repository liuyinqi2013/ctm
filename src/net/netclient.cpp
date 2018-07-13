#include "netclient.h"
#include <vector>

namespace ctm
{
	bool CNetTcpClient::Init()
	{

		if(!m_Socket.IsValid())
		{
			ERROR_LOG("m_Socket INVALID!");
			return false;
		}

		//设置空闲时间为30分钟
		if (!m_Socket.SetKeepAlive(600))
		{
			ERROR_LOG("errcode = %d, errmsg = %s!", m_Socket.GetErrCode(), m_Socket.GetErrMsg().c_str());
			return false;
		}

		if (!m_Socket.Connect(m_strServerIp, m_iPort))
		{
			ERROR_LOG("Connect %s:%d failed!", m_strServerIp.c_str(), m_iPort);
			return false;
		}

		FD_ZERO(&m_readFds);
		FD_SET(m_Socket.GetSock(), &m_readFds);

		return true;
		
	}

	int CNetTcpClient::Run()
	{
		int iRet = 0;
		int len  = 0;
		char buf[4096 + 1] = {0};
		std::vector<std::string> vecOutput;
		while(1)
		{
			fd_set fds = m_readFds;
			iRet = select(m_Socket.GetSock() + 1, &fds, NULL, NULL, NULL);
			if (iRet > 0)
			{
				len = m_Socket.Recv(buf, sizeof(buf) - 1);
				if (len > 0)
				{
					buf[len] = '\0';
					m_Context.GetCompletePack(m_Socket.GetSock(), std::string(buf, len), vecOutput);
					for(int i = 0; i < vecOutput.size(); ++i)
					{
						m_recvQueue.Push(vecOutput[i]);
					}
				}
				else
				{
					sleep(1);

					if (!m_bNeedReConn) break;
						
					while(!m_Socket.ReConnect())
					{
						ERROR_LOG("ReConnect %s:%d failed!", m_strServerIp.c_str(), m_iPort);
						sleep(2);
					}
					
					FD_ZERO(&m_readFds);
					FD_SET(m_Socket.GetSock(), &m_readFds);
					SendNetPack(m_connSendMsg);
				}
			}
			else if (iRet == 0)
			{
			}
			else
			{
			}
		}
		
		return 0;
	}

	void CNetTcpClient::ShutDown()
	{
		//关闭监控线程
		Stop();

		m_Socket.Close();
	}
}
