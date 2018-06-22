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
				player->m_dask = this;
				++m_playerCount;

				CJoinGameS2C msg;
				player->CopyTo(msg.m_player);
				BroadCast(&msg);

				CPlayerArrayMsg plaryArray;
				CPlayerMsg other;
				for (int j = 0; j < i; ++j)
				{
					m_playerArray[j]->CopyTo(other);
					plaryArray.Push(other);
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
}
