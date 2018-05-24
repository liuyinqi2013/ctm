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
		std::sort(m_cards, m_cards + 54);
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
		std::sort(vecCards.begin(), vecCards.end(), Compare);
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
}

