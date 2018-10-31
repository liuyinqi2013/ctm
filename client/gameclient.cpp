#include "gameclient.h"
#include <fstream>
#include <string.h>
#include <iostream>


CClientPlayer::CClientPlayer() :
	m_openId("0000000"),
	m_userName("Player"),
	m_headerImageUrl("www.doudizhu.com"),
	m_daskPos(-1),
	m_status(1)
{
}

CGameClient::CGameClient() :
	m_zhuangPos(-1),
	m_stop(false)
{
}

CGameClient::~CGameClient()
{
}

bool CGameClient::Init()
{
	FUNC_BEG();
	
	m_tcpClient.SetServerIp(m_serverIp);
	m_tcpClient.SetServerPort(m_port);
	m_tcpClient.SetEndFlag("[---@end@---]");
	if (!m_tcpClient.Init())
	{
		DEBUG_LOG("initialization tcp client failed!");
		return false;
	}
	
	m_tcpClient.Start();

	CLoginMsgC2S loginMsg;

	loginMsg.m_openId = m_clientPlayer.m_openId;
	loginMsg.m_userName = m_clientPlayer.m_userName;
	loginMsg.m_headerImageUrl = m_clientPlayer.m_headerImageUrl;

	DEBUG_LOG("send msg : %s", loginMsg.ToString().c_str());
	m_tcpClient.SendNetPack(loginMsg.ToString());
	m_tcpClient.SetConnSendMsg(loginMsg.ToString());
	m_tcpClient.SetNeedReConn(true);

	DEBUG_LOG("initialization ok!");

	FUNC_END();

	return true;
}

void CGameClient::Run()
{
	DEBUG_LOG("Run Begin !");
	while (1)
	{
		std::string buf = m_tcpClient.GetNetPack();
		DEBUG_LOG("Recv : %s", buf.c_str());

		Json::Value root;
		Json::Reader reader;
		if (reader.parse(buf, root))
		{
			CMsg* pMsg = CreateMsg(root["type"].asInt());
			pMsg->FromJson(root);
			pMsg->TestPrint();
			HandleMsg((CGameMsg*)pMsg);
			DestroyMsg(pMsg);

			if (m_stop) break;
		}
	}

	DEBUG_LOG("Run End !");
}

void CGameClient::HandleMsg(CGameMsg * pMsg)
{
	FUNC_BEG();
	
	switch(pMsg->m_iType)
	{
	case MSG_GAME_LOGIN_S2C:
		HandleLoginMsgS2C((CLoginMsgS2C*)pMsg);
		break;
	case MSG_GAME_LOGOUT_S2C:
		HandleLogoutMsgS2C((CLogoutMsgS2C*)pMsg);
		break;
	case MSG_GAME_PLAYER_ARRAY:
		HandlePlayerArrayMsgS2C((CPlayerArrayMsg*)pMsg);
		break;
	case MSG_GAME_JOIN_GAME_S2C:
		HandleJoinGameS2C((CJoinGameS2C*)pMsg);
		break;
	case MSG_GAME_GAME_BEGIN_S2C:
		HandleGameBeginS2C((CGameBeginS2C*)pMsg);
		break;
	case MSG_GAME_CALL_DIZHU_S2C:
		HandleCallDiZhuS2C((CCallDiZhuS2C*)pMsg);
		break;
	case MSG_GAME_OUT_CARD_S2C:
		HandleOutCardsS2C((COutCardsS2C*)pMsg);
		break;
	case MSG_GAME_GAME_OVER_S2C:
		HandleGameOverS2C((CGameOverS2C*)pMsg);
		break;
	case MSG_GAME_GAME_OVER_OPT:
		HandleGameOverOpt((CGameOverOpt*)pMsg);
		break;
	case MSG_GAME_GAME_INFO_S2C:
		HandleGameInfoS2C((CGameInfoS2C*)pMsg);
		break;
	default :
		break;
	}
			
	FUNC_END();
}

void CGameClient::HandleLoginMsgS2C(CLoginMsgS2C * pMsg)
{
	FUNC_BEG();

	CJoinGameC2S msg;

	msg.m_openId = m_clientPlayer.m_openId;
	SendMSG(&msg);

	FUNC_END();
}

void CGameClient::HandleLogoutMsgS2C(CLogoutMsgS2C * pMsg)
{
	FUNC_BEG();

	m_tcpClient.SetNeedReConn(false);
	m_stop = true;
	
	FUNC_END();
}

void CGameClient::HandleJoinGameS2C(CJoinGameS2C * pMsg)
{
	FUNC_BEG();
	
	if (m_clientPlayer.m_openId == pMsg->m_player.m_openId)
	{
		m_clientPlayer = pMsg->m_player;
	}
	else
	{
		m_otherPlayers[pMsg->m_player.m_openId] = pMsg->m_player;
		m_posOpenIdMap[pMsg->m_player.m_daskPos] = pMsg->m_player.m_openId;
	}

	FUNC_END();
}

void CGameClient::HandlePlayerArrayMsgS2C(CPlayerArrayMsg * pMsg)
{
	FUNC_BEG();
	
	for (int i = 0; i < pMsg->m_playerArray.size(); i++)
	{
		m_otherPlayers[pMsg->m_playerArray[i].m_openId]  = pMsg->m_playerArray[i];
		m_posOpenIdMap[pMsg->m_playerArray[i].m_daskPos] = pMsg->m_playerArray[i].m_openId;
	}

	FUNC_END();
}


void CGameClient::HandleGameBeginS2C(CGameBeginS2C * pMsg)
{
	FUNC_BEG();

	m_handCards = pMsg->m_handVec;
	
	DEBUG_LOG("my pos : %d", m_clientPlayer.m_daskPos);
	DEBUG_LOG("call pos : %d", pMsg->m_callPos);
	
	if (pMsg->m_callPos == m_clientPlayer.m_daskPos)
	{
		OptCallZhuang(1);
	}
	
	FUNC_END();
}

void CGameClient::HandleCallDiZhuS2C(CCallDiZhuS2C * pMsg)
{
	FUNC_BEG();

	if (pMsg->m_zhuangPos == -1)
	{
		if (pMsg->m_nextCallPos == m_clientPlayer.m_daskPos)
		{
			OptCallZhuang(pMsg->m_score);
		}
		else
		{
			if (pMsg->m_callPos != m_clientPlayer.m_daskPos)
			{
				DEBUG_LOG("%s call di zhu %d score", m_otherPlayers[pMsg->m_callOpenId].m_userName.c_str(), pMsg->m_score);
			}
		}
	}
	else 
	{
		if (pMsg->m_zhuangPos == m_clientPlayer.m_daskPos)
		{
			DEBUG_LOG("I am king @-@");
			for (int i = 0; i < pMsg->m_daskCardVec.size(); ++i)
			{
				m_handCards.push_back(pMsg->m_daskCardVec[i]);
				Sort1(m_handCards);
			}
			
			OutCards();
		}
		else
		{
			DEBUG_LOG("%s is king *-*", m_otherPlayers[m_posOpenIdMap[pMsg->m_zhuangPos]].m_userName.c_str());
			DEBUG_LOG("I am a farmer .-.");
			ShowHandCards();
		}
		
		
	}

	FUNC_END();

}

void CGameClient::HandleOutCardsS2C(COutCardsS2C * pMsg)
{
	FUNC_BEG();
	
	if (pMsg->m_outPos != m_clientPlayer.m_daskPos)
	{
		if (pMsg->m_outCardVec.size() == 0)
		{
			DEBUG_LOG("%s No out", m_otherPlayers[m_posOpenIdMap[pMsg->m_outPos]].m_userName.c_str());
		}
		else
		{
			DEBUG_LOG("%s out : ¡¾%s¡¿ ", m_otherPlayers[m_posOpenIdMap[pMsg->m_outPos]].m_userName.c_str(), CardsTypeToCN(pMsg->m_lastOutCardType).c_str());
		}
	}
	else if (pMsg->m_outPos == m_clientPlayer.m_daskPos)
	{
		if (pMsg->m_errCode != 0)
		{
			DEBUG_LOG("error code %d", pMsg->m_errCode);
			DEBUG_LOG("error msg %s", pMsg->m_errMsg.c_str());
			ShowCards(pMsg->m_lastOutCardVec);
			OutCards();
		}
		else
		{
			m_handCards = pMsg->m_handVec;
			ShowHandCards();
		}
		
		return;
	}

	if (pMsg->m_nextOutPos == m_clientPlayer.m_daskPos)
	{
		ShowCards(pMsg->m_lastOutCardVec);
		OutCards();
	}

	FUNC_END();
}


void CGameClient::HandleGameOverS2C(CGameOverS2C * pMsg)
{
	if ((m_zhuangPos == m_clientPlayer.m_daskPos && pMsg->m_winer == 1) ||
		(m_zhuangPos != m_clientPlayer.m_daskPos && pMsg->m_winer == 2))
	{
		DEBUG_LOG("I win");
	}
	else
	{
		DEBUG_LOG("I failed");
	}

	OptReady();
}

void CGameClient::HandleGameOverOpt(CGameOverOpt * pMsg)
{
	FUNC_END();
	
	if (pMsg->m_optPos != m_clientPlayer.m_daskPos)
	{
		string openid = m_posOpenIdMap[pMsg->m_optPos];
		DEBUG_LOG("%s %s game", openid.c_str(), (pMsg->m_opt == 1 ? "Ready" : "Quit"));
	}

	FUNC_END();
}


void CGameClient::HandleGameInfoS2C(CGameInfoS2C * pMsg)
{
	FUNC_BEG();

	m_zhuangPos = pMsg->m_zhuangPos;
	m_handCards = pMsg->m_handCards;
	
	m_otherPlayers.clear();
	for (int i = 0; i < pMsg->m_players.size(); ++i)
	{
		if (m_clientPlayer.m_openId == pMsg->m_players[i].m_openId)
		{
			m_clientPlayer = pMsg->m_players[i];
		}
		else
		{
			m_otherPlayers[pMsg->m_players[i].m_openId]  = pMsg->m_players[i];
			m_posOpenIdMap[pMsg->m_players[i].m_daskPos] = pMsg->m_players[i].m_openId;
		}
	}

	if (pMsg->m_currOptPos == m_clientPlayer.m_daskPos)
	{
		switch(pMsg->m_gameStatus)
		{
		case 0:
			break;
		case 1:
			OptCallZhuang(pMsg->m_maxScore);
			break;
		case 2:
			ShowCards(pMsg->m_lastOutCards);
			OutCards();
			break;
		case 3:
			break;
		case 4:
			OptReady();
			break;
		}
	}
	
	FUNC_END();
}



void CGameClient::SendMSG(CGameMsg * pMsg)
{
	pMsg->m_openId = m_clientPlayer.m_openId;
	m_tcpClient.SendNetPack(pMsg->ToString());
}

void CGameClient::ShowHandCards()
{
	cout<<"Total card num : "<<m_handCards.size()<<endl;
	ShowCards(m_handCards);

}

void CGameClient::GetOutCards(std::vector<CCard> & outCards)
{
	do
	{
		string input;
		cout<<"Please out cards[-1|num,num...]:"<<endl;
		cin>>input;
		if (input == "-1")
		{
			break;
		}
		
		std::vector<string> items;
		CutString(input, items, ",");
		int i = 0;
		for (; i < items.size(); ++i)
		{
			int index = S2I(items[i]);
			if (index < 0 || index > m_handCards.size())
			{
				break;
			}
			else
			{
				outCards.push_back(m_handCards[index]);
			}
		}

		if (i == items.size())
			break;
		else
			cout<<"Input format error!"<<endl;
	}
	while(1);
}


void CGameClient::OutCards()
{
	COutCardsC2S outCardsC2S;
	
	do
	{
		outCardsC2S.m_outCardVec.clear();
		ShowHandCards();
		outCardsC2S.m_outPos = m_clientPlayer.m_daskPos;
		GetOutCards(outCardsC2S.m_outCardVec);
		if (outCardsC2S.m_outCardVec.size() == 0)
		{
			DEBUG_LOG("I No out cards");
			break;
		}
		else
		{
			int type = CardsType(outCardsC2S.m_outCardVec);
			DEBUG_LOG("I out : ¡¾%s¡¿", CardsTypeToCN(type).c_str());
			ShowCards(outCardsC2S.m_outCardVec);
			
			if (CARDS_TYPE_UNKNOWN != type)
				break;
		}
	}
	while(1);
		
	SendMSG(&outCardsC2S);
}


void CGameClient::OptCallZhuang(int currScore)
{
	FUNC_BEG();
	
	CCallDiZhuC2S callDizhu;
	callDizhu.m_score = -1;
	callDizhu.m_callPos = m_clientPlayer.m_daskPos;
	while ((callDizhu.m_score < currScore || callDizhu.m_score > 3) && callDizhu.m_score != 0)
	{
		cout<<"call di zhu [0,"<<currScore<<"-3] : "<<endl;
		cin>>callDizhu.m_score;
	}
			
	SendMSG(&callDizhu);

	FUNC_END();
}


void CGameClient::OptReady()
{
	FUNC_BEG();

	do
	{
		int input;
		string buf;
		cout<<"Your choose [1-restart or 2-quit]:"<<endl;
		cin>>buf;

		input = S2I(buf);
		if (input != 1 && input != 2)
		{
			cout<<"invalid option"<<endl;
			continue;
		}

		CGameOverOpt gameOverOpt;
		gameOverOpt.m_opt = input;
		gameOverOpt.m_optPos = m_clientPlayer.m_daskPos;
		SendMSG(&gameOverOpt);
		break;
	}
	while(1);
	
	FUNC_END();
}






