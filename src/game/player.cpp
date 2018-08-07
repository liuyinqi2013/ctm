#include "player.h"
#include "dask.h"
#include "net/netmsg.h"
#include "common/log.h"

namespace ctm
{
	CPlayer::CPlayer() :
		m_daskPos(-1),
		m_dask(NULL),
		m_status(0),
		m_daskId(-1),
		m_manager(false)
		
	{
	
	}

	CPlayer::CPlayer(const CPlayer & other) :
		m_openId(other.m_openId),
		m_playerName(other.m_playerName),
		m_headerImageUrl(other.m_headerImageUrl),
		m_sock(other.m_sock),
		m_dask(other.m_dask),
		m_daskPos(other.m_daskPos),
		m_status(other.m_status)
		m_manager(false)
	{
	}
	
	CPlayer::~CPlayer()
	{
	
	}

	void CPlayer::HandleGameMsg(CGameMsg* pGameMsg)
	{
		if (m_dask) m_dask->HandleGameMSG(pGameMsg);
	}

	void CPlayer::Print()
	{
		DEBUG_LOG("m_sock = %u", m_sock.GetSock());
		DEBUG_LOG("m_openId = %s", m_openId.c_str());
		DEBUG_LOG("m_playerName = %s", m_playerName.c_str());
		DEBUG_LOG("m_headerImageUrl = %s", m_headerImageUrl.c_str());
		DEBUG_LOG("m_daskPos = %u", m_daskPos);
		DEBUG_LOG("m_daskId  = %u", m_daskId);
		DEBUG_LOG("m_status  = %u", m_status);
	}

	void CPlayer::Copy(const CPlayer & other)
	{
		m_openId = other.m_openId;
		m_playerName = other.m_playerName;
		m_headerImageUrl = other.m_headerImageUrl;
		m_sock = other.m_sock;
		m_dask = other.m_dask;
		m_daskPos = other.m_daskPos;
		m_status = other.m_status;	
	}
	
	void CPlayer::CopyTo(CPlayerItem & msg) const
	{
		msg.m_openId = m_openId;
		msg.m_userName = m_playerName;
		msg.m_headerImageUrl = m_headerImageUrl;
		msg.m_daskPos = m_daskPos;
		msg.m_status  = m_status;
		msg.m_daskId  = m_daskId;
	}

	void CPlayer::FormMsg(const CPlayerItem & msg)
	{
		m_openId = msg.m_openId;
		m_playerName = msg.m_userName;
		m_headerImageUrl = msg.m_headerImageUrl;
		m_daskPos = msg.m_daskPos;
		m_status  = msg.m_status;
		m_daskId  = msg.m_daskId;
	}
	
	Json::Value CPlayer::ToJson()
	{
		Json::Value value;
		
		value["openId"] = m_openId;
		value["playerName"] = m_playerName;
		value["headerImageUrl"] = m_headerImageUrl;
		value["daskPos"] = m_daskPos;
		value["status"]  = m_status;
		value["daskId"] = m_daskId;
		
		return value;
	}

	void CPlayer::FromJson(const Json::Value& json)
	{
		m_openId = json["openId"].asString();
		m_playerName = json["playerName"].asString();
		m_headerImageUrl = json["headerImageUrl"].asString();
		m_daskPos = json["daskPos"].asInt();
		m_status = json["status"].asInt();
		m_daskId = json["daskId"].asInt();
	}

	void CPlayer::SendMSG(CGameMsg* pGameMsg)
	{	
		if (1 == m_status)
		{
			pGameMsg->m_openId = m_openId;
			m_sock.Send(PackNetData(pGameMsg->ToString()));
		}
	}

	CPlayerItem CPlayer::ToPlayerItem() const
	{
		CPlayerItem playerItem;
		this->CopyTo(playerItem);

		return playerItem;
	}

	void CPlayer::SendGameInfo()
	{	
		if (!m_dask) return;
		CGameInfoS2C gameinfo;
		gameinfo.m_currOptPos = m_dask->m_currOptPos;
		gameinfo.m_lastOptPos = m_dask->m_lastOptPos;
		gameinfo.m_zhuangPos  = m_dask->m_zhuangPos;
		gameinfo.m_gameStatus = m_dask->m_gameStatus;
		gameinfo.m_maxScore   = m_dask->m_callMaxScore;

		for (int i = 0; i < DASK_MAX_PLAYERS; i++)
		{
			if (m_dask->m_playerArray[i])
				gameinfo.m_players.push_back(m_dask->m_playerArray[i]->ToPlayerItem());
		}

		for (int i = 0; i < m_dask->m_handCardsArray[m_daskPos].size(); i++)
		{
			gameinfo.m_handCards.push_back(*m_dask->m_handCardsArray[m_daskPos][i]);
		}

		for (int i = 0; i < m_dask->m_lastOutCards.size(); i++)
		{
			gameinfo.m_lastOutCards.push_back(m_dask->m_lastOutCards[i]);
		}

		SendMSG(&gameinfo);
		
	}
}
