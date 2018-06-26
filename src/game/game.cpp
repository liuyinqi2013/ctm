#include "game.h"

namespace ctm
{

	CGame::CGame() :
		m_gameName("doudizhuo"),
		m_playerNum(3)	
	{
	}
	
	CGame::~CGame()
	{
	}
		
	void CGame::ToDeal(std::vector<CCard*> handCardsArray[], std::vector<CCard*> & daskCard)
	{
		m_pokerCards.Shuffle();
		for (int i = 0; i < 17; ++i)
		{
			handCardsArray[0].push_back(m_pokerCards.m_cards + i * 3);
			handCardsArray[1].push_back(m_pokerCards.m_cards + i * 3 + 1);
			handCardsArray[2].push_back(m_pokerCards.m_cards + i * 3 + 2);
		}

		daskCard.push_back(m_pokerCards.m_cards + 51);
		daskCard.push_back(m_pokerCards.m_cards + 52);
		daskCard.push_back(m_pokerCards.m_cards + 53);	
	}

	void CGame::DeleteOutCards(std::vector<CCard*> & handCards, const std::vector<CCard> & outCards)
	{
		for (int i = 0 ; i < outCards.size(); ++i)
		{
			std::vector<CCard*>::iterator it = handCards.begin();
			for (; it != handCards.end(); ++it)
			{
				if (**it == outCards[i])
				{
					handCards.erase(it);
					break;
				}
			}
		}
	}

	bool HaveCards(std::vector<CCard*> & handCards, const std::vector<CCard> & outCards)
	{
		for (int i = 0 ; i < outCards.size(); ++i)
		{
			std::vector<CCard*>::iterator it = handCards.begin();
			for (; it != handCards.end(); ++it)
			{
				if (**it == outCards[i])
				{
					break;
				}
			}

			if (it == handCards.end()) return false;
		}

		return true;
	}
	
}
