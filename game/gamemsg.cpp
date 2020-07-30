#include "gamemsg.h"

namespace ctm
{
	const Json::Value& CGameMsg::ToJson()
	{
		CMsg::ToJson();
		m_root["openId"]  = m_openId;
		m_root["errCode"] = m_errCode;
		m_root["errMsg"]  = m_errMsg;
		return m_root;
	}

	void CGameMsg::FromJson(const Json::Value& json)
	{
		CMsg::FromJson(json);
		m_openId  = json["openId"].asString();
		m_errCode = json["errCode"].asInt();
		m_errMsg  = json["errMsg"].asString();
	}
		
	void CGameMsg::TestPrint()
	{
		CMsg::TestPrint();

		DEBUG_LOG("CGameMsg sock = %u",   m_sock);
		DEBUG_LOG("CGameMsg openId = %s", m_openId.c_str());
	}

	
	const Json::Value CPlayerItem::ToJson()
	{
		Json::Value m_root;
		m_root["openId"]   = m_openId;
		m_root["userName"] = m_userName;
		m_root["headerImageUrl"] = m_headerImageUrl;
		m_root["sex"] = m_sex;
		m_root["daskPos"] = m_daskPos;
		m_root["daskId"] = m_daskId;
		m_root["status"] = m_status;
		
		return m_root;
	}

	void CPlayerItem::FromJson(const Json::Value& json)
	{
		m_openId = json["openId"].asString();
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
		for (size_t i = 0; i < m_playerArray.size(); ++i)
		{
			array.append(m_playerArray[(int)i].ToJson());
		}
		
		m_root["playerArray"] = array;

		return m_root;
	}

	void CPlayerArrayMsg::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);
		m_playerArray.clear();
		CPlayerItem player;
		for (size_t i = 0; i < json["playerArray"].size(); ++i)
		{
			player.FromJson(json["playerArray"][(int)i]);
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

	const Json::Value& CLogoutMsgC2S::ToJson()
	{
		m_root["userName"] = m_userName;

		CGameMsg::ToJson();
		
		return m_root;
	}

	void CLogoutMsgC2S::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);

		m_userName = json["userName"].asString();	
	}

	const Json::Value& CLogoutMsgS2C::ToJson()
	{
		m_root["userName"] = m_userName;
		m_root["opt"] = m_opt;
		CGameMsg::ToJson();
		
		return m_root;
	}

	void CLogoutMsgS2C::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);

		m_openId = json["openId"].asString();
		m_userName = json["userName"].asString();
		m_opt = m_root["opt"].asInt();
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
		for (size_t i = 0; i < m_handVec.size(); ++i)
		{
			handCards.append(m_handVec[(int)i].ToJson());
		}
		m_root["handCards"] = handCards;

		return m_root;
	}

	void CGameBeginS2C::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);

		m_callPos = json["callPos"].asInt();

		m_handVec.clear();
		CCard card;
		for (size_t i = 0; i < json["handCards"].size(); ++i)
		{
			card.FromJson(json["handCards"][(int)i]);
			m_handVec.push_back(card);
		}
	}

	const Json::Value& CCallDiZhuC2S::ToJson()
	{
		CGameMsg::ToJson();

		m_root["score"] = m_score;
		m_root["callPos"] = m_callPos;
		return m_root;
	}

	void CCallDiZhuC2S::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);

		m_score = json["score"].asInt();
		m_callPos = json["callPos"].asInt();
	}

	const Json::Value& CCallDiZhuS2C::ToJson()
	{
		CGameMsg::ToJson();

		m_root["score"] = m_score;
		m_root["zhuangPos"] = m_zhuangPos;
		m_root["callOpenId"] = m_callOpenId;
		m_root["callPos"] = m_callPos;
		m_root["nextCallPos"] = m_nextCallPos;

		Json::Value daskCards;
		for (size_t i = 0; i < m_daskCardVec.size(); ++i)
		{
			daskCards.append(m_daskCardVec[(int)i].ToJson());
		}
		m_root["daskCards"] = daskCards;
		return m_root;
	}

	void CCallDiZhuS2C::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);

		m_score = json["score"].asInt();
		m_zhuangPos = json["zhuangPos"].asInt();
		m_callOpenId = json["callOpenId"].asString();
		m_callPos = json["callPos"].asInt();
		m_nextCallPos = json["nextCallPos"].asInt();
		
		m_daskCardVec.clear();
		CCard card;
		for (size_t i = 0; i < json["daskCards"].size(); ++i)
		{
			card.FromJson(json["daskCards"][(int)i]);
			m_daskCardVec.push_back(card);
		}
			
	}

	const Json::Value& COptS2C::ToJson()
	{
		CGameMsg::ToJson();

		m_root["optType"] = m_optType;
		m_root["optOpenId"] = m_optOpenId;
		
		return m_root;
	}

	void COptS2C::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);

		m_optType = json["optType"].asInt();
		m_optOpenId = json["optOpenId"].asString();
	}

	const Json::Value& COutCardsC2S::ToJson()
	{
		CGameMsg::ToJson();

		m_root["outPos"] = m_outPos;
		Json::Value daskCards;
		for (size_t i = 0; i < m_outCardVec.size(); ++i)
		{
			daskCards.append(m_outCardVec[(int)i].ToJson());
		}
		m_root["outCards"] = daskCards;
		return m_root;
	}

	void COutCardsC2S::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);

		m_outPos = json["outPos"].asInt();
		
		m_outCardVec.clear();
		CCard card;
		for (size_t i = 0; i < json["outCards"].size(); ++i)
		{
			card.FromJson(json["outCards"][(int)i]);
			m_outCardVec.push_back(card);
		}
			
	}

	const Json::Value& COutCardsS2C::ToJson()
	{
		CGameMsg::ToJson();

		m_root["outPos"] = m_outPos;
		m_root["outOpenId"] = m_outOpenId;
		m_root["nextOutPos"] = m_nextOutPos;
		Json::Value daskCards;
		for (size_t i = 0; i < m_outCardVec.size(); ++i)
		{
			daskCards.append(m_outCardVec[(int)i].ToJson());
		}
		m_root["outCards"] = daskCards;
		
		Json::Value lastCards;
		for (size_t i = 0; i < m_lastOutCardVec.size(); ++i)
		{
			lastCards.append(m_lastOutCardVec[(int)i].ToJson());
		}
		m_root["lastOutCards"] = lastCards;

		m_root["lastOutCardType"] = m_lastOutCardType;

		Json::Value handCards;
		for (size_t i = 0; i < m_handVec.size(); ++i)
		{
			handCards.append(m_handVec[(int)i].ToJson());
		}
		m_root["handCards"] = handCards;
		
		return m_root;
	}

	void COutCardsS2C::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);

		m_outPos = json["outPos"].asInt();
		m_outOpenId = json["outOpenId"].asString();
		m_nextOutPos = json["nextOutPos"].asInt();
		m_outCardVec.clear();
		m_lastOutCardVec.clear();
		CCard card;
		for (size_t i = 0; i < json["outCards"].size(); ++i)
		{
			card.FromJson(json["outCards"][(int)i]);
			m_outCardVec.push_back(card);
		}
		for (size_t i = 0; i < json["lastOutCards"].size(); ++i)
		{
			card.FromJson(json["lastOutCards"][(int)i]);
			m_lastOutCardVec.push_back(card);
		}

		m_lastOutCardType = json["lastOutCardType"].asInt();

		m_handVec.clear();
		for (size_t i = 0; i < json["handCards"].size(); ++i)
		{
			card.FromJson(json["handCards"][(int)i]);
			m_handVec.push_back(card);
		}
		
	}

	const Json::Value& CGameOverS2C::ToJson()
	{
		CGameMsg::ToJson();

		m_root["gameScore"] = m_gameScore;
		m_root["bombCount"] = m_bombCount;
		m_root["winer"] = m_winer;

		return m_root;
	}

	void CGameOverS2C::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);

		m_gameScore = json["gameScore"].asInt();
		m_bombCount = json["bombCount"].asInt();
		m_winer = json["winer"].asInt();	
	}

	const Json::Value& CGameOverOpt::ToJson()
	{
		CGameMsg::ToJson();

		m_root["opt"]    = m_opt;
		m_root["optPos"] = m_optPos;

		return m_root;
	}

	void CGameOverOpt::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);

		m_opt    = json["opt"].asInt();
		m_optPos = json["optPos"].asInt();	
	}

	
	const Json::Value& CGameInfoS2C::ToJson()
	{
		CGameMsg::ToJson();

		m_root["gameStatus"] = m_gameStatus;
		m_root["zhuangPos"]  = m_zhuangPos;
		m_root["currOptPos"] = m_currOptPos;
		m_root["lastOptPos"] = m_lastOptPos;
		m_root["m_maxScore"] = m_maxScore;
		
		Json::Value array;
		for (size_t i = 0; i < m_players.size(); ++i)
		{
			array.append(m_players[(int)i].ToJson());
		}
		m_root["players"] = array;

		Json::Value handCards;
		for (size_t i = 0; i < m_handCards.size(); ++i)
		{
			handCards.append(m_handCards[(int)i].ToJson());
		}
		m_root["handCards"] = handCards;

		Json::Value lastOutCards;
		for (size_t i = 0; i < m_lastOutCards.size(); ++i)
		{
			lastOutCards.append(m_lastOutCards[(int)i].ToJson());
		}
		m_root["lastOutCards"] = lastOutCards;

		return m_root;
	}

	void CGameInfoS2C::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);

		m_gameStatus = json["gameStatus"].asInt();
		m_zhuangPos  = json["zhuangPos"].asInt();
		m_currOptPos = json["currOptPos"].asInt();
		m_lastOptPos = json["lastOptPos"].asInt();
		m_maxScore   = json["m_maxScore"].asInt();

		m_players.clear();
		m_handCards.clear();
		m_lastOutCards.clear();
		
		CCard card;
		for (size_t i = 0; i < json["handCards"].size(); ++i)
		{
			card.FromJson(json["handCards"][(int)i]);
			m_handCards.push_back(card);
		}
		
		for (size_t i = 0; i < json["lastOutCards"].size(); ++i)
		{
			card.FromJson(json["lastOutCards"][(int)i]);
			m_lastOutCards.push_back(card);
		}
		
		CPlayerItem player;
		for (size_t i = 0; i < json["players"].size(); ++i)
		{
			player.FromJson(json["players"][(int)i]);
			m_players.push_back(player);
		}
	}

	REG_MSG(MSG_GAME_LOGIN_C2S, CLoginMsgC2S);
	REG_MSG(MSG_GAME_LOGIN_S2C, CLoginMsgS2C);
	REG_MSG(MSG_GAME_LOGOUT_C2S, CLogoutMsgC2S);
	REG_MSG(MSG_GAME_LOGOUT_S2C, CLogoutMsgS2C);
	REG_MSG(MSG_GAME_JOIN_GAME_C2S, CJoinGameC2S);
	REG_MSG(MSG_GAME_JOIN_GAME_S2C, CJoinGameS2C);

	REG_MSG(MSG_GAME_PLAYER_ARRAY, CPlayerArrayMsg);
	REG_MSG(MSG_GAME_GAME_BEGIN_S2C, CGameBeginS2C);
	REG_MSG(MSG_GAME_CALL_DIZHU_C2S, CCallDiZhuC2S);
	REG_MSG(MSG_GAME_CALL_DIZHU_S2C, CCallDiZhuS2C);
	REG_MSG(MSG_GAME_OPT_S2C, COptS2C);
	REG_MSG(MSG_GAME_OUT_CARD_C2S, COutCardsC2S);
	REG_MSG(MSG_GAME_OUT_CARD_S2C, COutCardsS2C);
	REG_MSG(MSG_GAME_GAME_OVER_S2C, CGameOverS2C);
	REG_MSG(MSG_GAME_GAME_OVER_OPT, CGameOverOpt);
	REG_MSG(MSG_GAME_GAME_INFO_S2C, CGameInfoS2C);

}
