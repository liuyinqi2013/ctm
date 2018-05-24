#ifndef CTM_GAME_GAME_H__
#define CTM_GAME_GAME_H__

#include <string>
#include <vector>
#include "Card.h"


namespace ctm
{
	class CDask;
	class CPlayer;
	
	class CGame
	{
	public:
		CGame();
		virtual ~CGame();

		void Start();

		void Over();

		bool Join(CPlayer* player);

		bool Quit(CPlayer* player);
		
	protected:
		std::string m_gameName;
		int m_status;
		int m_playerNum;
		CPokerCards m_pokerCards;
		CDask* m_dask;
	};
}

#endif

