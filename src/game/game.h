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

		void ToDeal(std::vector<CCard*> handCardsArray[], std::vector<CCard*> & daskCard);

		void DeleteOutCards(std::vector<CCard*> & handCards, const std::vector<CCard> & outCards);

		bool HaveCards(std::vector<CCard*> & handCards, const std::vector<CCard> & outCards);
		
	public:
		std::string m_gameName;
		int m_playerNum;
		CPokerCards m_pokerCards;
	};
}

#endif

