#ifndef CTM_GAME_PLAYER_H__
#define CTM_GAME_PLAYER_H__

#include <string>
#include "net/socket.h"

#include "json/json.h"
#include "json/json-forwards.h"

namespace ctm
{

	class CDask;
	class CGameMsg;
	class CPlayerItem;
	
	class CPlayer
	{
	
	friend bool operator == (const CPlayer & lhs, const CPlayer & rhs);
	
	public:
		CPlayer();
		CPlayer(const CPlayer & other);
		virtual ~CPlayer();

		void HandleGameMsg(CGameMsg* pGameMsg);

		void Print();

		void Copy(const CPlayer & other);
		
		void CopyTo(CPlayerItem & msg) const;

		void FormMsg(const CPlayerItem & msg);

		Json::Value ToJson();

		void FromJson(const Json::Value& json);

		void SendMSG(CGameMsg* pGameMsg);
		
		CPlayerItem ToPlayerItem() const;
	public:
		std::string m_openId;
		std::string m_playerName;
		std::string m_headerImageUrl;

		CSocket m_sock;
		CDask*  m_dask;
		int     m_daskPos;
		int     m_status;
		int     m_daskId;
	};

	inline bool operator == (const CPlayer & lhs, const CPlayer & rhs)
	{
		return lhs.m_openId == rhs.m_openId;
	}
}


#endif

