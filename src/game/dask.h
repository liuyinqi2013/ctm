#ifndef CTM_GAME_DASK_H__
#define CTM_GAME_DASK_H__

#include <vector>
#include "gamemsg.h"


#define  DASK_MAX_PLAYERS 4

namespace ctm
{

	class CGame;
	class CPlayer;
	class CCard;
	class CGameCenter;

	class CDask
	{
	public:
		CDask();
		~CDask();

		void Join(CPlayer* player);

		void Quit(CPlayer* player);

		void GameStart();

		void GameOver();

		bool IsFull() const;

		void Clear();

		void BroadCast(CGameMsg * pMsg);

		void BroadCastOutCardsMSG(COutCardsS2C * pMsg);

		void HandleGameMSG(CGameMsg * pMsg);
		
	private:
		void HandleCallDiZhuMSG(CCallDiZhuC2S * pMsg);
		void HandleOutCardsMSG(COutCardsC2S * pMsg);
		void HandleGameOverOptMSG(CGameOverOpt * pMsg);
		
	public:
		typedef enum GAME_STATUS
		{
			CAME_WAIT = 0,
			GAME_CALL = 1,
			GAME_RUN  = 2,
			GAME_OVER = 3,
			GAME_WAIT_READY = 4
		};
		
		int m_daskId;
		int m_capacity;
		CGame* m_game;
		int m_playerCount;
		CPlayer* m_playerArray[DASK_MAX_PLAYERS];
		std::vector<CCard*> m_handCardsArray[DASK_MAX_PLAYERS];
		std::vector<CCard*> m_daskCardsArray[DASK_MAX_PLAYERS];
		std::vector<CCard*> m_daskCards;
		std::vector<CCard> m_lastOutCards;
		int m_playerGameOverOpt[DASK_MAX_PLAYERS];
		int m_playerOutHandleCount[DASK_MAX_PLAYERS];
		int m_zhuangPos;
		int m_currOptPos;
		int m_lastOptPos;
		int m_gameStatus;  //0,1,2
		int m_callCount;
		int m_callMaxScore;
		int m_callMaxScorePos;
		int m_tatolOutBombCount;
		int m_readyCount;
		int m_quitCount;
		CGameCenter* m_gameCenter;	
	};
}


#endif

