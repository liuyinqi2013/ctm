#ifndef CTM_GAME_PLAYER_H__
#define CTM_GAME_PLAYER_H__

#include <string>
#include "net/socket.h"
#include "gamemsg.h"


namespace ctm
{

	class CDask;
	
	class CPlayer
	{
	public:
		CPlayer();
		CPlayer(const CPlayer & other);
		virtual ~CPlayer();

		void HandleGameMsg(CGameMsg* pGameMsg);

		void Print();

		void CopyTo(CPlayerMsg & msg);

		void FormMsg(const CPlayerMsg & msg);

		void SendMSG(CGameMsg* pGameMsg);
		
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
}


#endif

