#ifndef CTM_GAME_GAMEMSG_H__
#define CTM_GAME_GAMEMSG_H__

#include <string>
#include <vector>

#include "card.h"
#include "common/msg.h"

namespace ctm
{
	class CGameMsg : public CMsg
	{
	public:
		
		CGameMsg() : CMsg("0", MSG_GAME, "GameMsg"), m_sock(-1), m_errCode(0) { }
		CGameMsg(int type) : CMsg("0", type, "GameMsg"), m_sock(-1), m_errCode(0) { }
		CGameMsg(int type, const std::string& name) : CMsg("0", type, name), m_sock(-1), m_errCode(0) { }
		virtual ~CGameMsg() { }

		virtual const Json::Value& ToJson();

		virtual void FromJson(const Json::Value& json);

		virtual void TestPrint();
	public:
		int m_sock;
		int m_errCode;
		std::string m_errMsg;
		std::string m_openId;
	};

	class CPlayerItem 
	{
	public:
		CPlayerItem() { }
		virtual ~CPlayerItem() { }

		virtual const Json::Value ToJson();

		virtual void FromJson(const Json::Value& json);

	public:
		std::string m_openId;
		std::string m_userName;
		std::string m_headerImageUrl;
		int m_sex;
		int m_daskPos;
		int m_daskId;
		int m_status;
	};

	class CPlayerArrayMsg : public CGameMsg
	{
	public:
		CPlayerArrayMsg() : CGameMsg(MSG_GAME_PLAYER_ARRAY, "PlayerArrayMsg") { }
		virtual ~CPlayerArrayMsg() { }

		virtual const Json::Value& ToJson();

		virtual void FromJson(const Json::Value& json);

		void Push(const CPlayerItem & player) { m_playerArray.push_back(player); }
		
	public:
		std::vector<CPlayerItem> m_playerArray;
	};
	
	class CLoginMsgC2S : public CGameMsg
	{
	public:
		CLoginMsgC2S() : CGameMsg(MSG_GAME_LOGIN_C2S, "LoginMsgC2S") { }
		virtual ~CLoginMsgC2S() { }

		virtual const Json::Value& ToJson();

		virtual void FromJson(const Json::Value& json);
		
	public:
		std::string m_userName;
		std::string m_headerImageUrl;
	};

	
	class CLoginMsgS2C : public CGameMsg
	{
	public:
		CLoginMsgS2C() : CGameMsg(MSG_GAME_LOGIN_S2C, "LoginMsgS2C") { }
		virtual ~CLoginMsgS2C() { }

		virtual const Json::Value& ToJson();

		virtual void FromJson(const Json::Value& json);
		
	public:
		std::string m_gameInfo;
		int m_onlineCount;
	};

	class CLogOutMsgC2S : public CGameMsg
	{
	public:
		CLogOutMsgC2S() : CGameMsg(MSG_GAME_LOGOUT_C2S, "LogOutMsgC2S") { }
		virtual ~CLogOutMsgC2S() { }

		virtual const Json::Value& ToJson();

		virtual void FromJson(const Json::Value& json);
		
	public:
		std::string m_userName;
	};

	class CLogOutMsgS2C : public CGameMsg
	{
	public:
		CLogOutMsgS2C() : CGameMsg(MSG_GAME_LOGOUT_S2C, "LogOutMsgS2C") { }
		virtual ~CLogOutMsgS2C() { }

		virtual const Json::Value& ToJson();

		virtual void FromJson(const Json::Value& json);
		
	public:
		std::string m_userName;
	};

	class CJoinGameC2S : public CGameMsg
	{
	public:
		CJoinGameC2S() : CGameMsg(MSG_GAME_JOIN_GAME_C2S, "JoinGameC2S") { }
		virtual ~CJoinGameC2S() { }

		virtual const Json::Value& ToJson();

		virtual void FromJson(const Json::Value& json);
		
	};

	class CJoinGameS2C : public CGameMsg
	{
	public:
		CJoinGameS2C() : CGameMsg(MSG_GAME_JOIN_GAME_S2C, "JoinGameS2C") { }
		virtual ~CJoinGameS2C() { }

		virtual const Json::Value& ToJson();

		virtual void FromJson(const Json::Value& json);
		
	public:
		CPlayerItem m_player;
	};

	class CGameBeginS2C : public CGameMsg
	{
	public:
		CGameBeginS2C() : CGameMsg(MSG_GAME_GAME_BEGIN_S2C, "GameBeginS2C") { }
		virtual ~CGameBeginS2C() { }

		virtual const Json::Value& ToJson();

		virtual void FromJson(const Json::Value& json);
		
	public:
		int m_callPos;
		std::vector<CCard> m_handVec;
	};

	class CCallDiZhuC2S : public CGameMsg
	{
	public:
		CCallDiZhuC2S() : CGameMsg(MSG_GAME_CALL_DIZHU_C2S, "CallDiZhuC2S") { }
		virtual ~CCallDiZhuC2S() { }

		virtual const Json::Value& ToJson();

		virtual void FromJson(const Json::Value& json);
		
	public:
		int m_score;
		int m_callPos;
	};

	class CCallDiZhuS2C : public CGameMsg
	{
	public:
		CCallDiZhuS2C() : CGameMsg(MSG_GAME_CALL_DIZHU_S2C, "CallDiZhuC2S") { }
		virtual ~CCallDiZhuS2C() { }

		virtual const Json::Value& ToJson();

		virtual void FromJson(const Json::Value& json);
		
	public:
		int m_score;
		int m_zhuangPos;
		int m_callPos;
		int m_nextCallPos;
		std::string m_callOpenId;
		std::vector<CCard> m_daskCardVec;
	};

	class COptS2C : public CGameMsg
	{
	public:
		typedef enum opt_type
		{
			OPT_CALL_DIZHU = 1
		};
	public:
		COptS2C() : CGameMsg(MSG_GAME_OPT_S2C, "OptS2C") { }
		virtual ~COptS2C() { }

		virtual const Json::Value& ToJson();

		virtual void FromJson(const Json::Value& json);
		
	public:
		int m_optType;
		std::string m_optOpenId;
	};
		
	class COutCardsC2S : public CGameMsg
	{
	public:
		COutCardsC2S() : CGameMsg(MSG_GAME_OUT_CARD_C2S, "OutCardsC2S") { }
		virtual ~COutCardsC2S() { }

		virtual const Json::Value& ToJson();

		virtual void FromJson(const Json::Value& json);
		
	public:
		int m_outPos;
		std::vector<CCard> m_outCardVec;
	};

	class COutCardsS2C : public CGameMsg
	{
	public:
		COutCardsS2C() : CGameMsg(MSG_GAME_OUT_CARD_S2C, "OutCardsS2C") { }
		virtual ~COutCardsS2C() { }

		virtual const Json::Value& ToJson();

		virtual void FromJson(const Json::Value& json);
		
	public:
		int m_outPos;
		std::string m_outOpenId;
		int m_nextOutPos;
		std::vector<CCard> m_outCardVec;
		std::vector<CCard> m_lastOutCardVec;
		int m_lastOutCardType;
		std::vector<CCard> m_handVec;
	};

	class CGameOverS2C : public CGameMsg
	{
	public:
		CGameOverS2C() : CGameMsg(MSG_GAME_GAME_OVER_S2C, "GameOverS2C") { }
		virtual ~CGameOverS2C() { }

		virtual const Json::Value& ToJson();

		virtual void FromJson(const Json::Value& json);
		
	public:
		int m_gameScore;
		int m_bombCount;
		int m_winer;
	};
}

#endif

