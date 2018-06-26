#include "dask.h"
#include "game.h"
#include "player.h"
#include "card.h"
#include "center.h"
#include "net/netmsg.h"
#include "common/random.h"

namespace ctm
{

	CDask::CDask() :
		m_daskId(0),
		m_capacity(DASK_MAX_PLAYERS),
		m_game(NULL),
		m_playerCount(0),
		m_zhuangPos(0),
		m_currOptPos(0),
		m_gameStatus(0),
		m_callCount(0),
		m_callMaxScore(0),
		m_callMaxScorePos(0),
		m_gameCenter(NULL)
	{
		for(int i = 0; i < DASK_MAX_PLAYERS; i++)
		{
			m_playerArray[i] = NULL;
			m_playerOutBombCount[i]   = 0;
			m_playerOutHandleCount[i] = 0;
		}

		m_game = new CGame;
	}
	
	CDask::~CDask()
	{
		delete m_game;
	}

	void CDask::Join(CPlayer* player)
	{
		FUNC_BEG();
		
		if (NULL == player || NULL == m_game) return ;

		for (int i = 0; i < DASK_MAX_PLAYERS; i++)
		{
			if (NULL == m_playerArray[i])
			{
				m_playerArray[i] = player;
				player->m_daskPos = i;
				player->m_daskId = m_daskId;
				player->m_dask = this;
				++m_playerCount;

				CJoinGameS2C msg;
				//msg.m_player = *player;
				player->CopyTo(msg.m_player);
				BroadCast(&msg);

				CPlayerArrayMsg plaryArray;
				for (int j = 0; j < i; ++j)
				{
					plaryArray.Push(m_playerArray[j]->ToPlayerItem());
				}
				player->SendMSG(&plaryArray);
				break;
			}
		}

		if (IsFull())
		{
			GameStart();
		}

		FUNC_END();
	}

	void CDask::Quit(CPlayer* player)
	{
		
	}

	void CDask::GameStart()
	{
		FUNC_BEG();
		
		m_game->ToDeal(m_handCardsArray, m_daskCards);

		m_currOptPos = CRandom::Random(0, m_game->m_playerNum - 1);
		
		CGameBeginS2C gameBeginS2C;
		gameBeginS2C.m_callPos =  m_currOptPos;
		
		for(int i = 0; i < m_game->m_playerNum; ++i)
		{
			Sort1(m_handCardsArray[i]);

			gameBeginS2C.m_handVec.clear();
			for (int n = 0; n < m_handCardsArray[i].size(); ++n)
			{
				gameBeginS2C.m_handVec.push_back(*m_handCardsArray[i][n]);
			}
			m_playerArray[i]->SendMSG(&gameBeginS2C);
		}
		
		FUNC_END();
	}

	void CDask::GameOver()
	{
	}

	void CDask::Clear()
	{
		FUNC_BEG();
		
		for(int i = 0; i < DASK_MAX_PLAYERS; i++)
		{
			m_playerArray[i] = NULL;
			m_playerOutBombCount[i]   = 0;
			m_playerOutHandleCount[i] = 0;
			m_handCardsArray[i].clear();
			m_daskCardsArray[i].clear();
		}

		m_daskCards.clear();
		m_zhuangPos = 0;
		m_currOptPos = 0;
		m_playerCount = 0;
		m_gameStatus = 0;

		FUNC_END();	
	}

	bool CDask::IsFull() const
	{
		return (m_playerCount >= m_game->m_playerNum);
	}

	void CDask::BroadCast(CGameMsg * pMsg)
	{
		FUNC_BEG();
		
		for (int i = 0; i < DASK_MAX_PLAYERS; i++)
		{
			if (m_playerArray[i] != NULL)
			{
				m_playerArray[i]->SendMSG(pMsg);
			}
		}

		FUNC_END();
	}

	void CDask::HandleGameMSG(CGameMsg * pMsg)
	{
		FUNC_BEG();
		
		switch(pMsg->m_iType)
		{
		case MSG_GAME_CALL_DIZHU_C2S:
			HandleCallDiZhuMSG((CCallDiZhuC2S*)pMsg);
			break;
		case MSG_GAME_LOGOUT_C2S:
			break;
		case MSG_GAME_OUT_CARD_C2S:
			HandleOutCardsMSG((COutCardsC2S*)pMsg);
			break;
		default :
			break;
		}

		FUNC_END();
	}

	void CDask::HandleCallDiZhuMSG(CCallDiZhuC2S * pMsg)
	{
		FUNC_BEG();

		CCallDiZhuS2C callDizhuS2C;
		++m_callCount;
		if (m_callMaxScore < pMsg->m_score)
		{
			m_callMaxScore = pMsg->m_score;
			m_callMaxScorePos = pMsg->m_callPos;
		}
		
		callDizhuS2C.m_score = pMsg->m_score;
		callDizhuS2C.m_callPos = pMsg->m_callPos;
		callDizhuS2C.m_callOpenId = pMsg->m_openId;
		callDizhuS2C.m_zhuangPos = -1;

		if (pMsg->m_score == 3)
		{
			m_zhuangPos = pMsg->m_callPos;
			m_currOptPos = pMsg->m_callPos;
			callDizhuS2C.m_zhuangPos = pMsg->m_callPos;
			callDizhuS2C.m_nextCallPos = pMsg->m_callPos;
			for (int i = 0; i < m_daskCards.size(); ++i)
			{
				callDizhuS2C.m_daskCardVec.push_back(*m_daskCards[i]);
				m_handCardsArray[m_zhuangPos].push_back(m_daskCards[i]);
			}
			Sort1(m_handCardsArray[m_zhuangPos]);
		}
		else if (m_callCount == m_game->m_playerNum)
		{
			m_zhuangPos  = m_callMaxScorePos;
			m_currOptPos = m_callMaxScorePos;
			callDizhuS2C.m_zhuangPos = m_callMaxScorePos;
			callDizhuS2C.m_nextCallPos = m_callMaxScorePos;
			for (int i = 0; i < m_daskCards.size(); ++i)
			{
				callDizhuS2C.m_daskCardVec.push_back(*m_daskCards[i]);
				m_handCardsArray[m_zhuangPos].push_back(m_daskCards[i]);
			}
			Sort1(m_handCardsArray[m_zhuangPos]);
		}
		else
		{
			m_currOptPos =  (pMsg->m_callPos + 1) % 3;
			callDizhuS2C.m_nextCallPos = m_currOptPos;
		}

		BroadCast(&callDizhuS2C);
		
		FUNC_END();
	}

	void CDask::HandleOutCardsMSG(COutCardsC2S * pMsg)
	{
		FUNC_BEG();

		COutCardsS2C outCardsS2C;
		DEBUG_LOG("curr opt pos : %d, out pos :%d", m_currOptPos, pMsg->m_outPos);
		outCardsS2C.m_outPos = pMsg->m_outPos;
		outCardsS2C.m_outOpenId = m_playerArray[pMsg->m_outPos]->m_openId;
		outCardsS2C.m_outCardVec = pMsg->m_outCardVec;
		if (pMsg->m_outPos != m_currOptPos)
		{
			outCardsS2C.m_errCode = -1;
			outCardsS2C.m_errMsg  = "should not you out cards";
			m_playerArray[pMsg->m_outPos]->SendMSG(&outCardsS2C);
			return;
		}

		if (m_game->HaveCards(m_handCardsArray[pMsg->m_outPos], pMsg->m_outCardVec))
		{
			m_lastOutCards = pMsg->m_outCardVec;
		}
		else
		{
			DEBUG_LOG("no cards");
		}
		
		FUNC_END();
	}
}
