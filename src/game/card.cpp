#include "card.h"
#include "common/random.h"
#include "common/log.h"


namespace ctm
{
	CPokerCards::CPokerCards()
	{
		int i = 0;
		for (int type = CCard::CARD_TYPE_CLUBS; type <= CCard::CARD_TYPE_SPADES; ++type)
		{
			for (int num = CCard::CARD_NUM_2; num <= CCard::CARD_NUM_A; ++num)
			{
				m_cards[i].m_type = type;
				m_cards[i++].m_number = num;
			}
		}

		m_cards[i].m_type = CCard::CARD_TYPE_JOKER_BACK;
		m_cards[i++].m_number = CCard::CARD_NUM_JOKER;

		m_cards[i].m_type = CCard::CARD_TYPE_JOKER_RED;
		m_cards[i++].m_number = CCard::CARD_NUM_JOKER;
	}

	void CPokerCards::Sort()
	{
		std::sort(m_cards, m_cards + 54, CompareLt);
	}

	void CPokerCards::Sort1()
	{
		std::sort(m_cards, m_cards + 54, CompareLt1);
	}

	void CPokerCards::Shuffle()
	{
		int n;
		CCard p;
		CRandom::SetSeed();
		for (int i = 53; i > 0; --i)
		{
			n = CRandom::Random(0, i);
			p = m_cards[i];
			m_cards[i] = m_cards[n];
			m_cards[n] = p;
		}
	}

	std::string CPokerCards::ToString() const
	{
		std::string tmp;
		for (int i = 0; i < 54; ++i)
		{
			tmp += m_cards[i].ToString() + "\n";
		}

		return tmp;
	}
	
	void Sort(std::vector<CCard *> & vecCards)
	{
		std::sort(vecCards.begin(), vecCards.end(), ComparePtrLt);
	}

	void Sort1(std::vector<CCard *> & vecCards)
	{
		std::sort(vecCards.begin(), vecCards.end(), ComparePtrLt1);
	}

	void Sort(std::vector<CCard> & vecCards)
	{
		std::sort(vecCards.begin(), vecCards.end(), CompareLt1);
	}

	void Sort1(std::vector<CCard> & vecCards)
	{
		std::sort(vecCards.begin(), vecCards.end(), CompareLt1);
	}


	void Shuffle(std::vector<CCard *> & vecCards)
	{	
		int n;
		CCard * p;
		CRandom::SetSeed();
		for (int i = vecCards.size() - 1; i > 0; --i)
		{
			n = CRandom::Random(0, i);
			p = vecCards[i];
			vecCards[i] = vecCards[n];
			vecCards[n] = p;
		}
	}

	int CardsType(std::vector<CCard> & vecCards)
	{
		Sort1(vecCards);
		
		int count = vecCards.size();
		if (1 == count)
		{
			return CARDS_TYPE_SINGLE;
		}
		else if (2 == count)
		{
			if (Compare1(vecCards[0], vecCards[1]) == 0) 
			{
				if (vecCards[0].m_number == CCard::CARD_NUM_JOKER) return CARDS_TYPE_KING_BOMB;
				return CARDS_TYPE_PAIR;
			}
		}
		else if (3 == count)
		{
			return Compare1(vecCards[0], vecCards[2]) == 0 ?  CARDS_TYPE_THREE : CARDS_TYPE_UNKNOWN;
		}
		else if (4 == count)
		{
			if (Compare1(vecCards[0], vecCards[3]) == 0)
			{
				return CARDS_TYPE_BOMB;
			}
			else if (Compare1(vecCards[0], vecCards[2]) == 0)
			{
				return CARDS_TYPE_THREE_ONE;
			}
			else if (Compare1(vecCards[3], vecCards[1]) == 0)
			{
				return CARDS_TYPE_THREE_ONE;
			}
		}
		else if (5 == count)
		{
			if (Compare1(vecCards[0], vecCards[2]) == 0 &&
				Compare1(vecCards[3], vecCards[4]) == 0 )
			{
				return CARDS_TYPE_THREE_TWO;
			}
			else if (Compare1(vecCards[0], vecCards[1]) == 0 &&
				Compare1(vecCards[4], vecCards[2]) == 0 )
			{
				return CARDS_TYPE_THREE_TWO;
			}
			else if (IsShun(vecCards))
			{
				return CARDS_TYPE_SHUN;
			}
		}
		else if (6 == count)
		{
			if (Compare1(vecCards[0], vecCards[3]) == 0)
			{
				return CARDS_TYPE_FOUR_TWO;
			}
			else if (Compare1(vecCards[5], vecCards[2]) == 0)
			{
				return CARDS_TYPE_FOUR_TWO;
			}
			else if (IsShun(vecCards))
			{
				return CARDS_TYPE_SHUN;
			}
			else if (IsFly(vecCards))
			{
				return CARDS_TYPE_FLY;
			}
		}
		else
		{
			if (IsShun(vecCards))
			{
				return CARDS_TYPE_SHUN;
			}
			else if (IsFly(vecCards))
			{
				return CARDS_TYPE_FLY;
			}
		}

		return CARDS_TYPE_UNKNOWN;
	}

	int CardsCompare(const std::vector<CCard> & vecCards1, const std::vector<CCard> & vecCards2)
	{
	}

	bool IsShun(std::vector<CCard> & vecCards)
	{
		if (vecCards.size() < 5) return false;
		
		int i = 1;
		for (i = 1; i < vecCards.size(); ++i)
		{
			if (vecCards[i - 1].m_number - vecCards[i].m_number != -1)
			{
				return false;
			}
		}
		
 		return true;	
	}

	bool IsFly(std::vector<CCard> & vecCards)
	{
		if (vecCards.size() < 6 || vecCards.size() % 3 != 0) return false;

		int step = vecCards.size() / 3;
		int i = 0;
		for (i = 0; i < step - 1; ++i)
		{
			if (vecCards[3 * i].m_number - vecCards[3 * i + 3].m_number != -1 || Compare1(vecCards[3 * i], vecCards[3 * i + 2]) != 0)
			{
				return false;
			}
		}
		
 		return true;
	}
}

