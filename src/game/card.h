#ifndef CTM_GAME_CARD_H__
#define CTM_GAME_CARD_H__
#include "common/com.h"
#include <string>
#include <vector>
#include <algorithm>

namespace ctm
{
	class CCard
	{
	public:
		typedef enum card_num
		{
			CARD_NUM_2 = 0 ,	
			CARD_NUM_3 ,	
			CARD_NUM_4 ,	
			CARD_NUM_5 ,	
			CARD_NUM_6 ,	
			CARD_NUM_7 ,	
			CARD_NUM_8 ,	
			CARD_NUM_9 ,	
			CARD_NUM_10,
			CARD_NUM_J ,
			CARD_NUM_Q ,
			CARD_NUM_K ,
			CARD_NUM_A ,
			CARD_NUM_JOKER
		} CARD_NUM;

		typedef enum card_type
		{
			CARD_TYPE_CLUBS = 0,
			CARD_TYPE_DIAMONDS, 
			CARD_TYPE_HEARTS,
			CARD_TYPE_SPADES,
			CARD_TYPE_JOKER_BACK,
			CARD_TYPE_JOKER_RED
		} CARD_TYPE;
				
		CCard() : m_type(0), m_number(0)
		{
		}
		
		CCard(int number,  int type) : m_type(type), m_number(number)
		{
		}

		CCard(const CCard & other) : m_type(other.m_type), m_number(other.m_number)
		{
		}
		
		~CCard()
		{
		}

		CCard & operator = (const CCard & other)
		{
			m_type   = other.m_type;
			m_number = other.m_number;

			return *this;
		}

		int Compare(const CCard & other) const
		{
			return (m_type * 100 + m_number) - (other.m_type * 100 + other.m_number);
		}

		int Compare1(const CCard & other) const
		{
			return m_number - other.m_number;
		}

		std::string ToString() const
		{
			return I2S(m_type) + "-" + I2S(m_number); 
		}
		
	public:
		int m_type;
		int m_number;
	};

	class CPokerCards
	{
	public:
		CPokerCards();
		
		~CPokerCards() {}
		
		void Sort();
		
		void Shuffle();

		std::string ToString() const;
		
	public:
		CCard m_cards[54];
	};

	inline bool operator < (const CCard & lhs, const CCard & rhs)
	{
		return (lhs.Compare(rhs) < 0);
	}

	inline bool operator > (const CCard & lhs, const CCard & rhs)
	{
		return (lhs.Compare(rhs) > 0);
	}
	
	inline bool operator == (const CCard & lhs, const CCard & rhs)
	{
		return (lhs.Compare(rhs) == 0);
	}

	inline int Compare(const CCard * lhs, const CCard * rhs)
	{
		return lhs->Compare(*rhs);
	}

	void Sort(std::vector<CCard *> & vecCards);
	
	void Shuffle(std::vector<CCard *> & vecCards);

}

#endif