#include "ctm.h"
#include <iostream>
#ifndef WIN32
#include <unistd.h>
#else
#include <Windows.h>
#define sleep(n) Sleep((n) * 1000)
#endif
#include "common/singleton.h"
#include "common/com.h"
#include "thread/thread.h"
#include "thread/mutex.h"
#include "net/socket.h"

#include "netserver.h"

using namespace ctm;
using namespace std;

static int num = 0;
CMutex mutex;

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
	CMsgQueue msgQueue;
	CTcpNetServer server("127.0.0.1", 9999);
	server.SetMsgQueue(&msgQueue);
	if (!server.Init())
	{
		ERROR_LOG("Server init failed");
		return -1;
	}
	server.Start();
	char* send = "ctm:>";
	while(1)
	{
		CNetMsg* p = (CNetMsg*)msgQueue.Get(1);
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
		}
		usleep(5);
	}
	server.Join();
	return 0;
}