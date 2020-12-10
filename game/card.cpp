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
			n = CRandom::UIntRandom(0, i);
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
			n = CRandom::UIntRandom(0, i);
			p = vecCards[i];
			vecCards[i] = vecCards[n];
			vecCards[n] = p;
		}
	}

	int CardsType(std::vector<CCard> & vecCards)
	{
		int count = vecCards.size();
		
		if (0 == count) return CARDS_TYPE_UNKNOWN;
			
		Sort1(vecCards);
		
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
			else if (IsFlyTwo(vecCards)) 
			{
				return CARDS_TYPE_FLY_TWO;
			}
		}

		return CARDS_TYPE_UNKNOWN;
	}


	bool IsShun(std::vector<CCard> & vecCards)
	{
		if (vecCards.size() < 5) return false;
		
		for (size_t i = 1; i < vecCards.size(); ++i)
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
		
		for (size_t i = 2; i < vecCards.size(); i += 2)
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
		if (vecCards.size() < 8 || vecCards.size() % 4 != 0 ) return false;

		int array[15] = {0};
		CardsToArray(vecCards, array);
		
		size_t count = 0;
		size_t maxcount = 0;
		int i = 1;
		int j = 0;
		for (; i < 15; ++i, ++j)
		{
			if (array[i] >= 3 && array[j] >= 3)
			{
				++count;

				if (maxcount < count) maxcount = count;
			}
			else
			{
				count = 0;
			}
		}
		
		if ((maxcount + 1) * 4 == vecCards.size())
			return true;
		
 		return false;
	}

	bool IsFlyTwo(std::vector<CCard> & vecCards)
	{
		if (vecCards.size() < 10 || vecCards.size() % 5 != 0) return false;

		int array[15] = {0};
		CardsToArray(vecCards, array);

		size_t count = 0;
		size_t maxcount = 0;
		size_t paircount = 0;
		int i = 1;
		int j = 0;

		if(array[j] == 2) ++paircount;
		
		for (; i < 15; ++i, ++j)
		{
			if (array[i] >= 3 && array[j] >= 3)
			{
				++count;

				if (maxcount < count) maxcount = count; 
			}
			else
			{
				count = 0;
			}

			if(array[i] == 2) ++paircount;
		}
		
		if ((maxcount + 1) * 5 == vecCards.size() && paircount == maxcount + 1)
			return true;
		
 		return false;
	}

	int CardsCompare(std::vector<CCard> & vecCards1, std::vector<CCard> & vecCards2)
	{
		int iCardsType1 = CardsType(vecCards1);
		int iCardsType2 = CardsType(vecCards2);

		if (iCardsType1 != iCardsType2)
		{
			if (iCardsType1 == CARDS_TYPE_KING_BOMB || iCardsType1 == CARDS_TYPE_BOMB) return 1;
			else if (iCardsType2 == CARDS_TYPE_KING_BOMB || iCardsType2 != CARDS_TYPE_BOMB) return -1;
			else return -100;
		}
		else
		{
			if (vecCards1.size() != vecCards2.size()) return -100;

			if (iCardsType1 == CARDS_TYPE_SINGLE ||
				iCardsType1 == CARDS_TYPE_PAIR   ||
				iCardsType1 == CARDS_TYPE_SHUN   ||
				iCardsType1 == CARDS_TYPE_SHUN_PAIR)
			{
				return Compare1(vecCards1[0], vecCards2[0]);				
			}
			else if (iCardsType1 == CARDS_TYPE_THREE ||
				iCardsType1 == CARDS_TYPE_THREE_ONE  ||
				iCardsType1 == CARDS_TYPE_THREE_TWO  ||
				iCardsType1 == CARDS_TYPE_FOUR_TWO   ||
				iCardsType1 == CARDS_TYPE_FLY        ||
				iCardsType1 == CARDS_TYPE_FLY_TWO    ||
				iCardsType1 == CARDS_TYPE_BOMB)
			{
				CCard card1;
				CCard card2;
				for (size_t i = 0; i < vecCards1.size() - 2; ++i)
				{
					if (Compare1(vecCards1[i], vecCards1[i + 2]) == 0)
					{
						card1 = vecCards1[i];
						break;
					}
				}

				for (size_t i = 0; i < vecCards2.size() - 2; ++i)
				{
					if (Compare1(vecCards2[i], vecCards2[i + 2]) == 0)
					{
						card2 = vecCards2[i];
						break;
					}
				}

				return Compare1(card1, card2);
			}
		}
		
		return -100;
	}

	void ShowCards(const std::vector<CCard> & vecCards)
	{
		for (size_t i = 0; i < vecCards.size(); ++i)
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
		case CARDS_TYPE_FLY_TWO:
			return std::string("fly chack two");
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

	std::string CardsTypeToCN(int type)
	{
		switch(type)
		{
		case CARDS_TYPE_SINGLE:
			return std::string("����");
		case CARDS_TYPE_PAIR:
			return std::string("����");
		case CARDS_TYPE_THREE:
			return std::string("����");
		case CARDS_TYPE_THREE_ONE:
			return std::string("����һ");
		case CARDS_TYPE_THREE_TWO:
			return std::string("������");
		case CARDS_TYPE_FOUR_TWO:
			return std::string("�Ĵ���");
		case CARDS_TYPE_FLY:
			return std::string("�ɻ�");
		case CARDS_TYPE_FLY_TWO:
			return std::string("�ɻ�2");
		case CARDS_TYPE_SHUN:
			return std::string("˳��");
		case CARDS_TYPE_SHUN_PAIR:
			return std::string("����");
		case CARDS_TYPE_BOMB:
			return std::string("ը��");
		case CARDS_TYPE_KING_BOMB:
			return std::string("��ը");
		default :
			return std::string("δ֪");
		}
	}

	void CardsToArray(const std::vector<CCard> & vecCards, int array[15])
	{
		for (size_t i = 0; i < vecCards.size(); ++i)
		{
			if (vecCards[i].m_number >= 0 && vecCards[i].m_number < 15)
				array[vecCards[i].m_number] += 1; 
		}
	}

	void CardsToArray(const std::vector<CCard*> & vecCards, int array[15])
	{
		for (size_t i = 0; i < vecCards.size(); ++i)
		{
			if (vecCards[i]->m_number >= 0 && vecCards[i]->m_number < 15)
				array[vecCards[i]->m_number] += 1; 
		}
	}

	void TestCardsType()
	{
		std::vector<CCard> vec;
		CCard card;
		
		card.m_number = 0;
		vec.push_back(card);
		vec.push_back(card);

		card.m_number = 8;
		//vec.push_back(card);
		//vec.push_back(card);
		
		card.m_number = 10;
		
		//vec.push_back(card);
		//vec.push_back(card);

				
		card.m_number = 11;
		
		//vec.push_back(card);
		//vec.push_back(card);
		//vec.push_back(card);

		card.m_number = 8;		
		//vec.push_back(card);
		//vec.push_back(card);
		//card.m_number = 11;	
		//vec.push_back(card);
		//vec.push_back(card);

		DEBUG("card type : %s", CardsTypeToStr(CardsType(vec)).c_str());	
	}


}

