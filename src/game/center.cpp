#include "center.h"
#include "common/log.h"
#include "net/netserver.h"
#include "dask.h"
#include "player.h"
#include "game.h"


namespace ctm
{
	CGameCenter::CGameCenter(const std::string& ip, int port) :
		m_tcpNetServer(NULL),
		m_totalDaskNum(2),
		m_daskArray(NULL),
		m_ip(ip),
		m_port(port),
		m_waitDask(NULL)	
	{
	}

	CGameCenter::~CGameCenter()
	{
	}

	bool CGameCenter::Init()
	{
		FUNC_BEG();
		
		m_tcpNetServer =  new CTcpNetServer(m_ip, m_port);
		if (!m_tcpNetServer)
		{
			DEBUG_LOG("Create CTcpNetServer class failed!");
			return false;
		}

		if (!m_tcpNetServer->Init())
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
			m_daskArray[i].m_gameCenter = this;
			m_vecFreeDask.push_back(m_daskArray + i);
		}

		m_waitDask = m_vecFreeDask.front();

		m_vecFreeDask.pop_front();

		m_tcpNetServer->SetEndFlag("[---@end@---]");
		
		m_tcpNetServer->StartUp();

		FUNC_END();
		
		return true;
	}

	bool CGameCenter::Destroy()
	{
		FUNC_BEG();
		
		if (m_tcpNetServer)
		{
			m_tcpNetServer->ShutDown();
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

		FUNC_END();

		return true;
		
	}

	void CGameCenter::Run()
	{
		while (1)
		{
			CNetPack *p = m_tcpNetServer->GetNetPack();
			DEBUG_LOG("%s:%d recv : %s", p->ip, p->port, p->ibuf);
			if (p)
			{
				Json::Value root;
				Json::Reader reader;
				if (reader.parse(p->ibuf, root))
				{
					CMsg* pMsg = CreateMsg(root["type"].asInt());
					CGameMsg* pGameMsg = (CGameMsg*)pMsg;
					pGameMsg->m_sock  = p->sock;
					pGameMsg->FromJson(root);
					pGameMsg->TestPrint();
					if (pGameMsg->m_openId == "")
					{
						CSocket socket(p->sock, CSocket::SOCK_TYPE_STREAM);
						pGameMsg->m_errCode = 1;
						pGameMsg->m_errMsg  = "openid is null";
						socket.Send(PackNetData(pGameMsg->ToString()));
						continue;
					}
					HandleMsg(pGameMsg);
					DestroyMsg(pMsg);
				}
				else
				{
					CSocket socket(p->sock, CSocket::SOCK_TYPE_STREAM);
					socket.Send(PackNetData("Message format error!"));
				}	
			}

			m_tcpNetServer->Recycle(p);
			
		}
	}


	void CGameCenter::RecycleDask(CDask * dask)
	{
		if (m_daskArray <= dask && dask < m_daskArray + m_totalDaskNum)
		{
			m_vecFreeDask.push_back(dask);
		}
	}


	void CGameCenter::HandleMsg(CGameMsg * pMsg)
	{
		FUNC_BEG();
		
		switch(pMsg->m_iType)
		{
		case MSG_GAME_LOGIN_C2S:
			Login((CLoginMsgC2S*)pMsg);
			break;
		case MSG_GAME_LOGOUT_C2S:
			Logout((CLogOutMsgC2S*)pMsg);
			break;
		case MSG_GAME_JOIN_GAME_C2S:
			JoinGame((CJoinGameC2S*)pMsg);
			break;
		default :
			HandlePlayerMsg(pMsg);
			break;
		}

		FUNC_END();
	}

	void CGameCenter::Login(CLoginMsgC2S * pMsg)
	{
		FUNC_BEG();

		std::map<std::string, CPlayer*>::iterator it = m_mapOpenidPlayers.find(pMsg->m_openId);
		CPlayer* pPlayer = NULL;
		if (it != m_mapOpenidPlayers.end()) //可能掉线，重新登录。
		{
			pPlayer = it->second;
		}
		else
		{
			pPlayer = new CPlayer;
		}

		pPlayer->m_sock = CSocket(pMsg->m_sock, CSocket::SOCK_TYPE_STREAM);
		pPlayer->m_openId = pMsg->m_openId;
		pPlayer->m_playerName = pMsg->m_userName;
		pPlayer->m_headerImageUrl = pMsg->m_headerImageUrl;

		pPlayer->Print();
		m_mapOpenidPlayers[pPlayer->m_openId] = pPlayer;

		CLoginMsgS2C msg;
		msg.m_gameInfo ="dou di zhu";
		msg.m_onlineCount = m_mapOpenidPlayers.size();
		pPlayer->SendMSG(&msg);
		
		DEBUG_LOG("player count : %d", m_mapOpenidPlayers.size());

		FUNC_END();
	}

	void CGameCenter::Logout(CLogOutMsgC2S * pMsg)
	{	
		FUNC_BEG();

		FUNC_END();
	}

	void CGameCenter::JoinGame(CJoinGameC2S * pMsg)
	{
		FUNC_BEG();

		if (m_waitDask->IsFull())
		{
			if (m_vecFreeDask.size() > 0)
			{
				m_waitDask = m_vecFreeDask.front();
				m_vecFreeDask.pop_front();
			}
			else
			{
				return;
			}
		}

		m_waitDask->Join(m_mapOpenidPlayers[pMsg->m_openId]);

		FUNC_END();
	}


	void CGameCenter::HandlePlayerMsg(CGameMsg * pMsg)
	{
		FUNC_BEG();
		
		std::map<std::string, CPlayer*>::iterator it = m_mapOpenidPlayers.find(pMsg->m_openId);
		if (it != m_mapOpenidPlayers.end())
		{
			it->second->HandleGameMsg(pMsg);
		}
		
		FUNC_END();
	}
}
