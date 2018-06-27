#include "card.h"
#include "common/random.h"
#include "common/log.h"
#include <map>
#include <iostream>

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
				Compare1(vecCards[2], vecCards[4]) == 0 )
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
			else if (Compare1(vecCards[1], vecCards[4]) == 0)
			{
				return CARDS_TYPE_FOUR_TWO;
			}
			else if (Compare1(vecCards[2], vecCards[5]) == 0)
			{
				return CARDS_TYPE_FOUR_TWO;
			}
			else if (IsShun(vecCards))
			{
				return CARDS_TYPE_SHUN;
			}
			else if (IsShunPair(vecCards))
			{
				return CARDS_TYPE_SHUN_PAIR;
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
			else if (IsShunPair(vecCards))
			{
				return CARDS_TYPE_SHUN_PAIR;
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
		
		for (int i = 1; i < vecCards.size(); ++i)
		{
			if (vecCards[i].m_number - vecCards[i - 1].m_number != 1)
			{
				return false;
			}
		}
		
 		return true;	
	}

	bool IsShunPair(std::vector<CCard> & vecCards)
	{
		if (vecCards.size() < 6) return false;
		
		for (int i = 2; i < vecCards.size(); i += 2)
		{
			if (vecCards[i].m_number - vecCards[i - 2].m_number != 1 || 
				vecCards[i - 1].m_number != vecCards[i - 2].m_number || 
				vecCards[i].m_number != vecCards[i + 1].m_number)
			{
				return false;
			}
		}
		
 		return true;
	}

	bool IsFly(std::vector<CCard> & vecCards)
	{
		if (vecCards.size() < 8 || (vecCards.size() % 4 != 0 && vecCards.size() % 5 != 0)) return false;

		std::map<int, int> cardCountMap;

		std::map<int, int>::iterator it;
		for (int i = 0; i < vecCards.size(); ++i)
		{
			it = cardCountMap.find(vecCards[i].m_number);
			if (it == cardCountMap.end()) 
				cardCountMap[vecCards[i].m_number] = 1;
			else
				cardCountMap[vecCards[i].m_number] += 1;
		}

		it = cardCountMap.begin();
		std::map<int, int>::iterator it1 = it;
		int count = 0;
		int maxcount = 0;
		int paircount = 0;
		if (it->second == 2) ++paircount;
		for (++it1; it1 != cardCountMap.end(); ++it, ++it1)
		{
			if (it->second >= 3 && it1->second >= 3 && it1->first - it->first == 1)
			{
				++count;
			}
			else
			{
				if (maxcount < count) maxcount = count;
				count = 0;
			}

			if (it1->second == 2) ++paircount;
		}

		if ((maxcount + 1) * 4 == vecCards.size() || ((maxcount + 1) * 5 == vecCards.size() && paircount == maxcount + 1))
			return true;
		
 		return false;
	}

	void ShowCards(const std::vector<CCard> & vecCards)
	{
		for (int i = 0; i < vecCards.size(); ++i)
		{
			std::cout<<i<<":"<<vecCards[i].ToString()<<std::endl;
		}
		
	}

	std::string CardsTypeToStr(int type)
	{
		switch(type)
		{
		case CARDS_TYPE_SINGLE:
			return std::string("single");
		case CARDS_TYPE_PAIR:
			return std::string("pair");
		case CARDS_TYPE_THREE:
			return std::string("three");
		case CARDS_TYPE_THREE_ONE:
			return std::string("three plus one");
		case CARDS_TYPE_THREE_TWO:
			return std::string("three plus two");
		case CARDS_TYPE_FOUR_TWO:
			return std::string("four plus two");
		case CARDS_TYPE_FLY:
			return std::string("fly chack");
		case CARDS_TYPE_SHUN:
			return std::string("shun");
		case CARDS_TYPE_SHUN_PAIR:
			return std::string("shun pair");
		case CARDS_TYPE_BOMB:
			return std::string("bomb");
		case CARDS_TYPE_KING_BOMB:
			return std::string("king bomb");
		default :
			return std::string("unknown");
		}
		
	}
}

