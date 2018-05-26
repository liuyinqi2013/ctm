#include "center.h"
#include "dask.h"
#include "player.h"
#include "common/log.h"

namespace ctm
{
	CGameCenter::CGameCenter(const std::string& ip, int port) :
		m_tcpNetServer(NULL),
		m_totalDaskNum(2),
		m_daskArray(NULL),
		m_ip(ip),
		m_port(port)
		
	{
	}

	CGameCenter::~CGameCenter()
	{
	}

	bool CGameCenter::Init()
	{
		m_tcpNetServer =  new CTcpNetServer(m_ip, m_port);
		if (!m_tcpNetServer)
		{
			DEBUG_LOG("Create CTcpNetServer class failed!");
			return false;
		}

		if (m_tcpNetServer.Init())
		{
			DEBUG_LOG("Net server init failed!");
			return false;
		}

		m_daskArray = new CDask[m_totalDaskNum];
		if (!m_daskArray)
		{
			DEBUG_LOG("Create CDask array failed!");
			return false;
		}

		for (int i = 0; i < m_totalDaskNum; ++i)
		{
			m_daskArray[i].m_daskId = i + 1;
			m_vecFreeDask.push_back(m_daskArray + i);
		}

		m_tcpNetServer->StartUp();
		
		return true;
	}

	bool CGameCenter::Destroy()
	{
		if (m_tcpNetServer)
		{
			m_tcpNetServer->ShotDown();
			delete m_tcpNetServer;
			m_tcpNetServer = NULL;
			DEBUG_LOG("Destroy Net Server!");
		}

		if (m_daskArray)
		{
			delete [] m_daskArray;
			m_daskArray = NULL;

			m_vecFreeDask.clear();
		}
		
	}

	void CGameCenter::Run()
	{
		CNetPack *p = m_tcpNetServer->GetNetPack();
		if (p)
		{
			Json::Value root;
			Json::Reader reader;
			if (reader.parse(p->ibuf, root))
			{
				CMsg* pMsg = CreateMsg(root["type"].asInt());
				pMsg->FromJson(root);
				HandleMsg(pMsg);
			}
			else
			{
				CSocket socket(p->sock, SOCK_TYPE_STREAM);
				socket.Send(Pack("Message format error!"));
			}	
		}

		m_tcpNetServer->Recycle(p);
	}


	void CGameCenter::HandleMsg(CMsg * pMsg)
	{
		switch(pMsg->m_iType)
		{
		case MSG_GAME_LOGIN:
			Login((CLoginMsg*)pMsg);
			break;
		case MSG_GAME_LOGOUT:
			Logout((CLogOutMsg*)pMsg);
			break;
		default :
			
		}
	}

	void CGameCenter::Login(CLoginMsg * pMsg)
	{
		std::map<std::string, CPlayer*>::iterator it = m_mapPlayers.find(pMsg->m_openId);
		CPlayer* pPlayer = NULL;
		if (it != m_mapPlayers.end()) //可能掉线，重新登录。
		{
			pPlayer = it->second;
			pPlayer.m_sock.Close();
		}
		else
		{
			pPlayer = new CPlayer;
		}

		pPlayer->m_sock = CSocket(pMsg->sock, SOCK_TYPE_STREAM);
		pPlayer->m_openId = pMsg->m_openId;
		pPlayer->m_playerName = pMsg->m_userName;
		pPlayer->m_headerImageUrl = pMsg->m_headerImageUrl;

		m_mapPlayers[pPlayer->m_openId] = pPlayer;
		

	}

	void CGameCenter::Logout(CLogOutMsg * pMsg)
	{	
		
	}
}