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

		void HandleGameMSG(CGameMsg * pMsg);
		void HandleCallDiZhuMSG(CCallDiZhuC2S * pMsg);
		void HandleOutCardsMSG(COutCardsC2S * pMsg);
		
	public:
		int m_daskId;
		int m_capacity;
		CGame* m_game;
		int m_playerCount;
		CPlayer* m_playerArray[DASK_MAX_PLAYERS];
		std::vector<CCard*> m_handCardsArray[DASK_MAX_PLAYERS];
		std::vector<CCard*> m_daskCardsArray[DASK_MAX_PLAYERS];
		std::vector<CCard*> m_daskCards;
		std::vector<CCard> m_lastOutCards;
		int m_playerOutBombCount[DASK_MAX_PLAYERS];
		int m_playerOutHandleCount[DASK_MAX_PLAYERS];
		int m_zhuangPos;
		int m_currOptPos;
		int m_lastOptPos;
		int m_gameStatus;
		int m_callCount;
		int m_callMaxScore;
		int m_callMaxScorePos;
		CGameCenter* m_gameCenter;	
	};
}


#endif

