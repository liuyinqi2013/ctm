#ifndef CTM_GAME_PLAYER_H__
#define CTM_GAME_PLAYER_H__

#include <vector>
#include "Card.h"

#define  DASK_MAX_PLAYERS 4

namespace ctm
{

	class CGame;
	class CPlayer;
	
	class CDask
	{
	public:
		CDask();
		~CDask();
		
	public:
		std::string m_daskId;
		int m_capacity;
		CGame* m_game;
		CPlayer* m_playerArray[DASK_MAX_PLAYERS];
	};
}


#endif

