#include "gamemsg.h"

namespace ctm
{

	const Json::Value& CGameMsg::ToJson()
	{
		CMsg::ToJson();
		m_root["openId"] = m_openId;
	}

	void CGameMsg::FromJson(const Json::Value& json)
	{
		CMsg::FromJson(json);
		m_openId = json["openId"].asString();
	}
		
	void CGameMsg::TestPrint()
	{
		CMsg::TestPrint();

		DEBUG_LOG("CGameMsg sock = %u",   m_sock);
		DEBUG_LOG("CGameMsg openId = %s", m_openId.c_str());
	}

	
	const Json::Value& CPlayerMsg::ToJson()
	{
		CGameMsg::ToJson();
		
		m_root["userName"] = m_userName;
		m_root["headerImageUrl"] = m_headerImageUrl;
		m_root["sex"] = m_sex;
		m_root["daskPos"] = m_daskPos;
		m_root["daskId"] = m_daskId;
		m_root["status"] = m_status;
		
		return m_root;
	}

	void CPlayerMsg::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);

		m_userName = json["userName"].asString();
		m_headerImageUrl = json["headerImageUrl"].asString();
		m_sex = json["sex"].asInt();
		m_daskPos = json["daskPos"].asInt();
		m_daskId  = json["daskId"].asInt();
		m_status  = json["status"].asInt();
	}

	const Json::Value& CPlayerArrayMsg::ToJson()
	{
		CGameMsg::ToJson();

		Json::Value array;
		for (int i = 0; i < m_playerArray.size(); ++i)
		{
			array.append(m_playerArray[i].ToJson());
		}
		
		m_root["playerArray"] = array;

		return m_root;
	}

	void CPlayerArrayMsg::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);
		m_playerArray.clear();
		CPlayerMsg player;
		for (int i = 0; i < json["playerArray"].size(); ++i)
		{
			player.FromJson(json["playerArray"][i]);
			m_playerArray.push_back(player);
		}
	}

	const Json::Value& CLoginMsgC2S::ToJson()
	{
		CGameMsg::ToJson();
		
		m_root["userName"] = m_userName;
		m_root["headerImageUrl"] = m_headerImageUrl;
		
		return m_root;
	}

	void CLoginMsgC2S::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);

		m_userName = json["userName"].asString();
		m_headerImageUrl = json["headerImageUrl"].asString();
		
	}

	const Json::Value& CLoginMsgS2C::ToJson()
	{
		CGameMsg::ToJson();

		m_root["gameInfo"] = m_gameInfo;
		m_root["onlineCount"] = m_onlineCount;

		return m_root;
	}

	void CLoginMsgS2C::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);
		
		m_gameInfo = json["gameInfo"].asString();
		m_onlineCount = json["onlineCount"].asInt();
	}

	const Json::Value& CLogOutMsgC2S::ToJson()
	{
		m_root["userName"] = m_userName;

		CGameMsg::ToJson();
		
		return m_root;
	}

	void CLogOutMsgC2S::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);

		m_userName = json["userName"].asString();	
	}

	const Json::Value& CLogOutMsgS2C::ToJson()
	{
		m_root["userName"] = m_userName;

		CGameMsg::ToJson();
		
		return m_root;
	}

	void CLogOutMsgS2C::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);

		m_openId = json["openId"].asString();
		m_userName = json["userName"].asString();	
	}

	const Json::Value& CJoinGameC2S::ToJson()
	{

		CGameMsg::ToJson();
		return m_root;
	}

	void CJoinGameC2S::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);	
	}

	const Json::Value& CJoinGameS2C::ToJson()
	{

		CGameMsg::ToJson();
		m_root["player"] = m_player.ToJson();
		return m_root;
	}

	void CJoinGameS2C::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);
		m_player.FromJson(json["player"]);
	}

	const Json::Value& CGameBeginS2C::ToJson()
	{
		CGameMsg::ToJson();

		m_root["callPos"] = m_callPos;
		
		Json::Value handCards;
		for (int i = 0; i < m_handVec.size(); ++i)
		{
			handCards.append(m_handVec[i].ToJson());
		}
		m_root["handCards"] = handCards;

		return m_root;
	}

	void CGameBeginS2C::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);

		m_callPos = m_root["callPos"].asInt();

		m_handVec.clear();
		CCard card;
		for (int i = 0; i < json["handCards"].size(); ++i)
		{
			card.FromJson(json["handCards"][i]);
			m_handVec.push_back(card);
		}
	}

	REG_MSG(MSG_GAME_LOGIN_C2S, CLoginMsgC2S);
	REG_MSG(MSG_GAME_LOGIN_S2C, CLoginMsgS2C);
	REG_MSG(MSG_GAME_LOGOUT_C2S, CLogOutMsgC2S);
	REG_MSG(MSG_GAME_LOGOUT_S2C, CLogOutMsgS2C);
	REG_MSG(MSG_GAME_JOIN_GAME_C2S, CJoinGameC2S);
	REG_MSG(MSG_GAME_JOIN_GAME_S2C, CJoinGameS2C);

	REG_MSG(MSG_GAME_PLAYER_INFO, CPlayerMsg);
	REG_MSG(MSG_GAME_PLAYER_ARRAY, CPlayerArrayMsg);
	REG_MSG(MSG_GAME_GAME_BEGIN_S2C, CGameBeginS2C);

}
