#ifndef CTM_GAME_GAMEMSG_H__
#define CTM_GAME_GAMEMSG_H__

#include <string>
#include <vector>
#include "common/msg.h"

namespace ctm
{
	class CGameMsg : public CMsg
	{
	public:
		
		CGameMsg() : CMsg(0, MSG_GAME, "GameMsg") { }
		CGameMsg(int type) : CMsg(0, type, "GameMsg") { }
		CGameMsg(int type, const std::string& name) : CMsg(0, type, name) { }
		virtual ~CGameMsg() { }
		
	public:
		int m_sock;
	};
	
	class CLoginMsg : public CGameMsg
	{
	public:
		CLoginMsg() : CGameMsg(MSG_GAME_LOGIN, "LoginMsg") { }
		virtual ~CLoginMsg() { }

		virtual const Json::Value& ToJson() const;

		virtual void FromJson(const Json::Value& json);
		
	public:
		std::string m_openId;
		std::string m_userName;
		std::string m_headerImageUrl;
	};

	class CLogOutMsg : public CGameMsg
	{
	public:
		CLogOutMsg() : CGameMsg(MSG_GAME_LOGOUT, "LogOutMsg") { }
		virtual ~CLogOutMsg() { }

		virtual const Json::Value& ToJson() const;

		virtual void FromJson(const Json::Value& json);
		
	public:
		std::string m_openId;
		std::string m_userName;
	};
}

#endif

