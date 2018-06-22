#ifndef CTM_GAME_CENTER_H__
#define CTM_GAME_CENTER_H__

#include <string>
#include <map>
#include <vector>
#include <list>


#include "gamemsg.h"

namespace ctm
{

	class CDask;
	class CPlayer;
	class CTcpNetServer;
	
	class CGameCenter
	{
	public:
		CGameCenter(const std::string& ip = "0.0.0.0", int port = 6666);
		~CGameCenter();

		bool Init();

		bool Destroy();

		void Run();

		void RecycleDask(CDask * dask);

	private:
		
		void Login(CLoginMsgC2S * pMsg);

		void Logout(CLogOutMsgC2S * pMsg);

		void JoinGame(CJoinGameC2S * pMsg);

		void HandleMsg(CGameMsg * pMsg);

		void HandlePlayerMsg(CGameMsg * pMsg);
		
	private:
		CTcpNetServer* m_tcpNetServer;
		int m_totalDaskNum;
		CDask*  m_daskArray;
		std::map<std::string, CPlayer*> m_mapOpenidPlayers;
		std::map<int, CPlayer*> m_mapSockPlayers;
		std::list<CDask*> m_vecFreeDask;
		std::string m_ip;
		int m_port;
		CDask* m_waitDask;
	};
}


#endif

