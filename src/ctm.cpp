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

#include "netserver.h"

using namespace ctm;
using namespace std;

static int num = 0;
CMutex mutex;

#define MAXFD 64

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

class TestSingleton : public CSingleton<TestSingleton>, public CThread
{
public:
	TestSingleton(const std::string& name) : CThread(name)
	{
	}
	
	void hello() { cout<<"hello"<<endl; }

protected:
	int Run()
	{
		cout<<"thread name : "<<GetName()<<endl;
		cout<<"thread status : "<<GetStatus()<<endl;
		cout<<"thread str2int : "<<S2I("123")<<endl;
		cout<<"thread int2str : "<<I2S(10)<<endl;
		cout<<"thread str2double : "<<S2D("123.23")<<endl;
		cout<<"thread double2str : "<<D2S(1.004)<<endl;
		
		while(1)
		{
			{
				CLockOwner owner(mutex);
				num++;
				cout<<"thread : "<<GetName()<<" num : "<<num<<endl;
			}
			sleep(1);
		}
		return 0;
	}
	
};

int main(int argc, char **argv)
{

	Daemon();

	CLog::GetInstance()->SetLogName("ctm");
	CLog::GetInstance()->SetLogPath("/opt/test/ctm/log");
	//CLog::GetInstance()->SetOnlyBack(true);
	
	CTcpNetServer server("0.0.0.0", 9999);
	if (!server.Init())
	{
		ERROR_LOG("Server init failed");
		return -1;
	}
	
	server.StartUp();
	
	char* send = "ctm:>";
	
	while(1)
	{
		DEBUG_LOG("Get a msg");
		CNetMsg* p = server.GetMsg();
		DEBUG_LOG("recv a msg");
		if (p) 
		{
			p->TestPrint();
			int len = p->m_sock.Send(send, strlen(send));
			if (len == -1) 
			{
				DEBUG_LOG("errcode = %d, errmsg = %s!", p->m_sock.GetErrCode(), p->m_sock.GetErrMsg().c_str());
			}
			DEBUG_LOG("ip = %s, port = %d, len = %d, send = %s", p->m_strIp.c_str(), p->m_iPort, len, send);
			delete p;
			p = NULL;
		}
		usleep(5);
	}
	server.Join();
	return 0;
}
