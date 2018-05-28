#ifndef CTM_GAME_CENTER_H__
#define CTM_GAME_CENTER_H__

#include <string>
#include <map>
#include <vector>

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

	private:
		
		void Login(CLoginMsg * pMsg);

		void Logout(CLogOutMsg * pMsg);

		void HandleMsg(CMsg * pMsg);
		
	private:
		CTcpNetServer* m_tcpNetServer;
		int m_totalDaskNum;
		CDask*  m_daskArray;
		std::map<std::string, CPlayer*> m_mapPlayers;
		std::vector<CDask*> m_vecFreeDask;
		std::string m_ip;
		int m_port;
	};
}


#endif

