#ifndef CTM_GAME_CARD_H__
#define CTM_GAME_CARD_H__
#include "common/com.h"
#include <string>
#include <vector>
#include <algorithm>

#include "json/json.h"
#include "json/json-forwards.h"


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
			CARD_NUM_JOKER = 14
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
		
		CCard(int type, int number) : m_type(type), m_number(number)
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
			return "[" + I2S(m_type) + "-" + I2S(m_number) + "]"; 
		}

		Json::Value ToJson()
		{
			Json::Value value;
			value["type"]   = m_type;
			value["number"] = m_number;

			return value;
		}

		void FromJson(const Json::Value& json)
		{
			m_type   = json["type"].asInt();
			m_number = json["number"].asInt();
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

		void Sort1();
		
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

	inline int Compare(const CCard & lhs, const CCard & rhs)
	{
		return lhs.Compare(rhs);
	}

	inline int Compare1(const CCard & lhs, const CCard & rhs)
	{
		return lhs.Compare1(rhs);
	}
		
	inline bool CompareLt(const CCard & lhs, const CCard & rhs)
	{
		return (lhs.Compare(rhs) < 0);
	}
		
	inline bool CompareLt1(const CCard & lhs, const CCard & rhs)
	{
		return (lhs.Compare1(rhs) < 0);
	}

	inline bool ComparePtrLt(const CCard * lhs, const CCard * rhs)
	{
		return (lhs->Compare(*rhs) < 0);
	}

	inline bool ComparePtrLt1(const CCard * lhs, const CCard * rhs)
	{
		return (lhs->Compare1(*rhs) < 0);
	}

	void Sort(std::vector<CCard *> & vecCards);

	void Sort1(std::vector<CCard *> & vecCards);

	void Sort(std::vector<CCard> & vecCards);

	void Sort1(std::vector<CCard> & vecCards);
	
	void Shuffle(std::vector<CCard *> & vecCards);

	enum cards_type
	{
		CARDS_TYPE_UNKNOWN = 0,
		CARDS_TYPE_SINGLE,
		CARDS_TYPE_PAIR,
		CARDS_TYPE_THREE,
		CARDS_TYPE_THREE_ONE,
		CARDS_TYPE_THREE_TWO,
		CARDS_TYPE_FOUR_TWO,
		CARDS_TYPE_FLY,
		CARDS_TYPE_FLY_TWO,
		CARDS_TYPE_SHUN,
		CARDS_TYPE_SHUN_PAIR,
		CARDS_TYPE_BOMB,
		CARDS_TYPE_KING_BOMB
	};

	int CardsType(std::vector<CCard> & vecCards);

	int CardsCompare(std::vector<CCard> & vecCards1, std::vector<CCard> & vecCards2);

	bool IsShun(std::vector<CCard> & vecCards);

	bool IsShunPair(std::vector<CCard> & vecCards);

	bool IsFly(std::vector<CCard> & vecCards);

	bool IsFlyTwo(std::vector<CCard> & vecCards);

	void ShowCards(const std::vector<CCard> & vecCards);

	std::string CardsTypeToStr(int type);

	std::string CardsTypeToCN(int type);

	void CardsToArray(const std::vector<CCard> & vecCards, int array[15]);

	void CardsToArray(const std::vector<CCard*> & vecCards, int array[15]);

	void TestCardsType();
}

#endif
