
#include "gameclient.h"


int main(int argc, char **argv)
{
	CLog::GetInstance()->SetLogName("test");
	CLog::GetInstance()->SetLogPath("/opt/test/ctm/log");

	CGameClient game_client;

	game_client.Init();

	game_client.Run();
		
	return 0;
}

