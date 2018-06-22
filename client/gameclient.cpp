#include "gameclient.h"
#include <fstream>
#include <string.h>

CGameClient::CGameClient()
{
}

CGameClient::~CGameClient()
{
}

bool CGameClient::Init()
{
	ReadIni();
	
	m_tcpClient.SetServerIp("127.0.0.1");
	m_tcpClient.SetServerPort(6666);
	m_tcpClient.SetEndFlag("[---@end@---]");
	if (!m_tcpClient.Init())
	{
		DEBUG_LOG("initialization tcp client failed!");
		return false;
	}
	
	m_tcpClient.Start();

	/*m_openId = "44509740";
	m_userName = "pandaliu";
	m_headerImageUrl = "www.baidu.com";
	*/

	CLoginMsgC2S loginMsg;

	loginMsg.m_openId = m_openId;
	loginMsg.m_userName = m_userName;
	loginMsg.m_headerImageUrl = m_headerImageUrl;

	m_tcpClient.SendNetPack(loginMsg.ToString());
	m_tcpClient.SetConnSendMsg(loginMsg.ToString());

	DEBUG_LOG("initialization ok!");

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
	case MSG_GAME_JOIN_GAME_S2C:
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

	msg.m_openId = m_openId;
	m_tcpClient.SendNetPack(msg.ToString());

	FUNC_END();
}

void CGameClient::ReadIni()
{
	FUNC_BEG();

	fstream fileIni("conf.ini",  std::fstream::in | std::fstream::out);

	char buf[512] = {0};
	string strBuf;
	string key;
	string value;
	while(!fileIni.eof())
	{
		fileIni.getline(buf, sizeof(buf) - 1);
		if (buf[0] == '#') continue;
		strBuf = buf;
		
		int pos = strBuf.find("=");
		
		if (pos == string::npos) continue;

		key = strBuf.substr(0, pos);
		value = strBuf.substr(pos + 1);

		if (key == "openid") m_openId = value;
		else if (key == "username") m_userName = value;
		else if (key == "headurl")  m_headerImageUrl = value;
	}

	int id = S2I(m_openId) + 1;
	string centent = "openid=" + I2S(id) + "\n";
		centent += "username=" + m_userName + "\n";
		centent += "headurl=" + m_headerImageUrl + "\n";

	DEBUG_LOG("openid : %s", m_openId.c_str());
	DEBUG_LOG("username : %s", m_userName.c_str());
	DEBUG_LOG("headurl : %s", m_headerImageUrl.c_str());

	//DEBUG_LOG("centent : %s", centent.c_str());
	ofstream out("conf.ini");
	out<<centent;
	
	FUNC_BEG();
}


