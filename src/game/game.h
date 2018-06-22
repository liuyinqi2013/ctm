#ifndef CTM_GAME_GAME_H__
#define CTM_GAME_GAME_H__

#include <string>
#include <vector>
#include "card.h"


namespace ctm
{
	class CDask;
	class CPlayer;
	
	class CGame
	{
	public:
		CGame();
		virtual ~CGame();

		virtual void ToDeal(std::vector<CCard*> handCardsArray[], std::vector<CCard*> & daskCard);
		
	public:
		std::string m_gameName;
		int m_playerNum;
		CPokerCards m_pokerCards;
	};
}

#endif

