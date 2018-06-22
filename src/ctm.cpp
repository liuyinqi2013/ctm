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

	CLog::GetInstance()->SetLogName("ctm");
	CLog::GetInstance()->SetLogPath("/opt/test/ctm/log");
	//CLog::GetInstance()->SetOnlyBack(true);


	CGameCenter center;

	if (center.Init())
	{
		return -1;
	}
	
	center.Run();
	/*
	server.SetEndFlag("[---@end@---]");
	
	if (!server.Init())
	{
		ERROR_LOG("Server init failed");
		return -1;
	}
	
	server.StartUp();
	
	char* send = "ctm:>";

	FILE* fp = NULL;
	unsigned long total_size = 0;
	while(1)
	{
		//DEBUG_LOG("Get a msg");

		
		CNetPack* pPackRecv = server.GetNetPack();
		if (pPackRecv)
		{
			//DEBUG_LOG("recv a msg");
			if (strncmp(pPackRecv->ibuf, "[--filename--]:", strlen("[--filename--]:"))== 0)
			{
				int len = strlen("[--filename--]:");
				DEBUG_LOG("filename = %s", string(pPackRecv->ibuf + len).c_str());
				fp = fopen(pPackRecv->ibuf + len, "w+b");
				total_size = 0;
			}
			else
			{
				if (fp)
				{
					if (strncmp(pPackRecv->ibuf, "[--end--]",  strlen("[--end--]")) == 0)
					{
						DEBUG_LOG("[file end]");
						fclose(fp);
						fp = NULL;
						DEBUG_LOG("file total size =%ul", total_size);
						
					}
					else
					{
						fwrite(pPackRecv->ibuf, 1, pPackRecv->ilen, fp);
						total_size += pPackRecv->ilen;
					}
				}
			}
			
			pPackRecv->olen = strlen(send);
			strncpy(pPackRecv->obuf, send, pPackRecv->olen);
			pPackRecv->obuf[pPackRecv->olen] = '\0';
			//pPackRecv->TestPrint();
			server.SendNetPack(pPackRecv);
		}	
	}
	
	server.Join();
	*/
	return 0;
}
