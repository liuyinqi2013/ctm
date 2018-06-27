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
	m_zhuangPos(-1)
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

	DEBUG_LOG("initialization ok!");

	FUNC_END();

	return true;
}

void CGameClient::Run()
{
	DEBUG_LOG("Run !");
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
		}
	}
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

	for (int i = 0; i < pMsg->m_handVec.size(); ++i)
	{
		m_handCards.push_back(pMsg->m_handVec[i]);
	}

	DEBUG_LOG("my pos : %d", m_clientPlayer.m_daskPos);
	DEBUG_LOG("call pos : %d", pMsg->m_callPos);
	
	if (pMsg->m_callPos == m_clientPlayer.m_daskPos)
	{
		CCallDiZhuC2S callDizhu;
		callDizhu.m_score = -1;
		callDizhu.m_callPos = m_clientPlayer.m_daskPos;
		while (callDizhu.m_score < 0 || callDizhu.m_score > 3)
		{
			cout<<"call di zhu [0-3] : "<<endl;
			cin>>callDizhu.m_score;
		}
		
		SendMSG(&callDizhu);
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
			CCallDiZhuC2S callDizhu;
			callDizhu.m_score = -1;
			callDizhu.m_callPos = m_clientPlayer.m_daskPos;
			while ((callDizhu.m_score < pMsg->m_score || callDizhu.m_score > 3) && callDizhu.m_score != 0)
			{
				cout<<"call di zhu [0,"<<pMsg->m_score<<"-3] : "<<endl;
				cin>>callDizhu.m_score;
			}
			
			SendMSG(&callDizhu);
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
			
			ShowHandCards();
			COutCardsC2S outCardsC2S;
			outCardsC2S.m_outPos = m_clientPlayer.m_daskPos;
			GetOutCards(outCardsC2S.m_outCardVec);
			if (outCardsC2S.m_outCardVec.size() == 0)
			{
				DEBUG_LOG("I No out cards");
			}
			else
			{
				int type = CardsType(outCardsC2S.m_outCardVec);
				DEBUG_LOG("I out : %s ", CardsTypeToStr(type).c_str());
				ShowCards(outCardsC2S.m_outCardVec);
			}
			
			SendMSG(&outCardsC2S);

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
			int type = CardsType(pMsg->m_outCardVec);
			DEBUG_LOG("%s out : %s ", m_otherPlayers[m_posOpenIdMap[pMsg->m_outPos]].m_userName.c_str(), CardsTypeToStr(type).c_str());
			ShowCards(pMsg->m_outCardVec);
		}
	}

	if (pMsg->m_nextOutPos == m_clientPlayer.m_daskPos)
	{
		ShowHandCards();
		COutCardsC2S outCardsC2S;
		outCardsC2S.m_outPos = m_clientPlayer.m_daskPos;
		GetOutCards(outCardsC2S.m_outCardVec);
		if (outCardsC2S.m_outCardVec.size() == 0)
		{
			DEBUG_LOG("I No out cards");
		}
		else
		{
			int type = CardsType(outCardsC2S.m_outCardVec);
			DEBUG_LOG("I out : %s ", CardsTypeToStr(type).c_str());
			ShowCards(outCardsC2S.m_outCardVec);
		}
		
		SendMSG(&outCardsC2S);
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
		cout<<"Please out cards[0|num,num...]:"<<endl;
		cin>>input;
		if (input == "0")
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






