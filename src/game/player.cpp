#include "player.h"
#include "dask.h"
#include "net/netmsg.h"
#include "common/log.h"

namespace ctm
{
	CPlayer::CPlayer() :
		m_daskPos(-1)
		
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
		pGameMsg->m_openId = m_openId;
		m_sock.Send(PackNetData(pGameMsg->ToString()));
	}

	CPlayerItem CPlayer::ToPlayerItem() const
	{
		CPlayerItem playerItem;
		this->CopyTo(playerItem);

		return playerItem;
	}
}
