
#include "gameclient.h"
#include "common/inifile.h"


int main(int argc, char **argv)
{
	CIniFile iniFile("conf.ini");

	if (!iniFile.Load())
	{
		DEBUG_LOG("load conf.ini error");
		return -1;
	}

	DEBUG_LOG("centont :\n%s", iniFile.ToString().c_str());
	
	CLog::GetInstance()->SetLogName(iniFile["logfile"].AsString());
	CLog::GetInstance()->SetLogPath(iniFile["logpath"].AsString());

	CGameClient game_client;

	game_client.m_clientPlayer.m_openId = iniFile["openid"].AsString();
	game_client.m_clientPlayer.m_userName = iniFile["username"].AsString();
	game_client.m_clientPlayer.m_headerImageUrl = iniFile["headurl"].AsString();

	iniFile["openid"] = iniFile["openid"].AsInt() + 1;
	iniFile["username"] = "Panda" + iniFile["openid"].AsString();
	iniFile.Save();
	
	game_client.Init(iniFile["serverip"].AsString(), iniFile["serverport"].AsInt());

	game_client.Run();
		
	return 0;
}

