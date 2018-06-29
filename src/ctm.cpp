#include "ctm.h"
#include <iostream>
#ifndef WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#else
#include <Windows.h>
#define sleep(n) Sleep((n) * 1000)
#endif
#include "common/com.h"
#include "common/log.h"
#include "common/singleton.h"
#include "common/string_tools.h"
#include "common/inifile.h"


#include "thread/thread.h"
#include "thread/mutex.h"
#include "net/socket.h"

#include "game/card.h"
#include "game/center.h"


#include "netserver.h"

using namespace ctm;
using namespace std;

static int num = 0;
CMutex mutex;

#define MAXFD 64

CTcpNetServer server("0.0.0.0", 9999);

void Daemon()
{
	pid_t pid = fork();
	if (pid < 0)
	{
		perror("fork failed!\n");
	}
	else if (pid > 0)
	{
		exit(0);
	}

	setsid();
	signal(SIGHUP, SIG_IGN);

	pid = fork();
	if (pid < 0)
	{
		perror("fork failed!\n");
	}
	else if (pid > 0)
	{
		exit(0);
	}

	chdir("/");
	umask(0);

	for (int i = 0; i < MAXFD; i++)
	{
		close(i);
	}

	int fd = open("/dev/null", O_RDWR);
	if (fd < 0)
	{
		perror("open /dev/null failed!\n");
	}
		
	dup2(fd, 0);
	dup2(fd, 1);
	dup2(fd, 2);
}

void Handle_PIPE(int sign)
{
	DEBUG_LOG("--- Recv sign SIGPIPE ---");
}

void Handle_INT(int sign)
{
	DEBUG_LOG("--- Recv sign SIGINT ---");
	exit(1);
}

void RegSignHandleFunc()
{
	signal(SIGPIPE, Handle_PIPE);
	signal(SIGINT, Handle_INT);
}


int main(int argc, char **argv)
{

	//Daemon();


	RegSignHandleFunc();

	CIniFile iniFile("conf.ini");

	if (!iniFile.Load())
	{
		DEBUG_LOG("load conf.ini error");
		return -1;
	}

	//DEBUG_LOG("centont :\n%s", iniFile.ToString().c_str());
	
	CLog::GetInstance()->SetLogName(iniFile["logfile"].AsString());
	CLog::GetInstance()->SetLogPath(iniFile["logpath"].AsString());

	//CLog::GetInstance()->SetOnlyBack(true);

	CGameCenter center;

	if (!center.Init(iniFile["serverip"].AsString(), iniFile["serverport"].AsInt()))
	{
		return -1;
	}
	
	center.Run();
	
	return 0;
}
