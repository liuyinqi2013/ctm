#include "gamemsg.h"

namespace ctm
{
	const Json::Value& CLoginMsg::ToJson() const
	{
		m_root["openId"] = m_openId;
		m_root["userName"] = m_userName;
		m_root["headerImageUrl"] = m_headerImageUrl;

		CGameMsg::ToJson();
		
		return m_root;
	}

	void CLoginMsg::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);

		m_openId = json["openId"].asString();
		m_userName = json["userName"].asString();
		m_headerImageUrl = json["headerImageUrl"].asString();
		
	}

	const Json::Value& CLogOutMsg::ToJson() const
	{
		m_root["openId"] = m_openId;
		m_root["userName"] = m_userName;

		CGameMsg::ToJson();
		
		return m_root;
	}

	void CLogOutMsg::FromJson(const Json::Value& json)
	{
		CGameMsg::FromJson(json);

		m_openId = json["openId"].asString();
		m_userName = json["userName"].asString();	
	}

}