#include "common/singleton.h"
#include "common/string_tools.h"
#include "common/time_tools.h"
#include "common/msg.h"
#include "common/com.h"
#include "common/log.h"
#include "common/random.h"

#include "net/socket.h"
#include "net/select.h"
#include "ipc/mmap.h"
#include "ipc/semaphore.h"
#include "ipc/sharememory.h"

#include <string.h>
#include <signal.h>


#include <iostream>
#include <ctype.h>
#include <iostream>

using namespace ctm;
using namespace std;

void ShowSign(int sign)
{
	DEBUG_LOG("Recv a sign : %d", sign);
}

void HandleSign()
{
	signal(SIGPIPE, ShowSign);
}
	
class TestSingleton : public CSingleton<TestSingleton>
{
public:
	void Hello() { cout<<"hello"<<endl; }
};

void TestStringFunc()
{
	string a("@@\r\n**}&\n@[@end@]");
	cout<<a<<endl;
	cout<<EndsWith(a, "@@")<<endl;
	cout<<StartsWith(a, "@@")<<endl;
	cout<<Trimmed(a)<<endl;
	cout<<Time()<<endl;
	cout<<UTime()<<endl;
	cout<<DateTime()<<endl;
	cout<<DateTime(TFMT_1)<<endl;
	cout<<DateTime(TFMT_2)<<endl;
	std::vector<std::string> vecOutput;
	CutString(a, vecOutput, "[@end@]", false);
	cout<<"item size = "<<vecOutput.size()<<endl;
	for (int i = 0; i < vecOutput.size(); ++i)
	{
		cout<<"item = "<<vecOutput[i]<<endl;
	}
}

void TestTcpClient()
{
	TcpClient client;
	if (!client.Connect("127.0.0.1", 9999))
	{
		cout<<"connect server failed!\n"<<endl;
		return ;
	}
	char buf[1024] = {0};
	int len = client.Recv(buf, 1024);
	if (len == -1)
	{
		cout<<"recv failed!\n"<<endl;
		return ;
	}
	buf[len] = '\0';
	cout<<"Recv : "<<buf<<endl;
	char* s = "hello server";
	client.Send(s, strlen(s));
	
}

void PrintHost(struct hostent* hostinfo)
{	
	DEBUG_LOG("official name of host : %s", hostinfo->h_name);
	string hostAliases;
	char** head = hostinfo->h_aliases;
	for(;*head ; ++head)
	{
		hostAliases += string(" ") + *head;
	}
	DEBUG_LOG("alias list : %s", hostAliases.c_str());
	
	if (hostinfo->h_addrtype == AF_INET)
	{
		DEBUG_LOG("host address type : AF_INET");
	}
	else
	{
		DEBUG_LOG("host address type : AF_INET6");
	}
	
	DEBUG_LOG("length of address : %d", hostinfo->h_length);

	char buf[128] = {0};
	string hostAddrs;
	head = hostinfo->h_addr_list;
	for(;*head ; ++head)
	{
		inet_ntop(hostinfo->h_addrtype,  *head, buf, sizeof(buf));
		hostAddrs += string(" ") + buf;
	}
	
	DEBUG_LOG("list of addresses  : %s", hostAddrs.c_str());

}

void TestGetHostByAddr()
{
	while(1)
	{
		DEBUG_LOG("Pealse input host ip:");
		char buf[128] = {0};
		cin.getline(buf, sizeof(buf));
		int size = cin.gcount();
		struct in_addr inaddr= {0};
		
		int ret = inet_pton(AF_INET, buf, &inaddr);
		if (ret <= 0)
		{
			DEBUG_LOG("Not in presentation format : %s", buf);
			continue;
		}
		
		struct hostent* hostinfo = gethostbyaddr((const char*)&inaddr, sizeof(inaddr), AF_INET);
		if (!hostinfo)
		{
			DEBUG_LOG("Couldn't resolve host ip : %s", buf);
			continue;
		}
		
		PrintHost(hostinfo);		
	}
	
}

void TestGetHostByName()
{
	while(1)
	{
		DEBUG_LOG("Pealse input host name:");
		char buf[128] = {0};
		cin.getline(buf, sizeof(buf));
		int size = cin.gcount();
		struct hostent* hostinfo = gethostbyname(buf);
		if (!hostinfo)
		{
			DEBUG_LOG("Couldn't resolve host name : %s", buf);
			continue;
		}
		
		PrintHost(hostinfo);		
	}
	
}


void TestGetAddrInfo()
{
	struct addrinfo addr = {0};
	addr.ai_family = AF_UNSPEC;
	addr.ai_socktype = SOCK_STREAM;
	addr.ai_flags = AI_NUMERICHOST;
	struct addrinfo *res, *p;
	if (0 != getaddrinfo("localhost", NULL, &addr, &res))
	{
		cout<<"getaddrinfo failed"<<endl;
	}

	for ( p = res; p != NULL; p = p->ai_next)
	{
		char ipbuf[128] = {0};
		if (p->ai_family == AF_INET)
		{
			cout<<"type AF_INET"<<endl;
			struct sockaddr_in *sa = (struct sockaddr_in *)p->ai_addr;
			inet_ntop(AF_INET, &(sa->sin_addr), ipbuf, 128);
		}
		else if (p->ai_family == AF_INET6)
		{
			cout<<"type AF_INET6"<<endl;
			struct sockaddr_in *sa = (struct sockaddr_in *)p->ai_addr;
			inet_ntop(AF_INET6, &(sa->sin_addr), ipbuf, 128);
		}
		cout<<"ip : "<<ipbuf<<endl;

		cout<<"canonname : "<<p->ai_canonname<<endl;
	}
	freeaddrinfo(res);
}

void TestSelect()
{
	TcpClient client;

	if (!client.Connect("127.0.0.1", 9999))
	{
		DEBUG_LOG("connect server failed : %d, %s!", client.GetErrCode(),  client.GetErrMsg().c_str());
		return ;
	}
 
	
	CSelect s;

	s.AddReadFd(client.GetSocket());
	s.AddReadFd(fileno(stdin));
	while (1)
	{
		struct timeval timeOut = { 5, 10 };

		int iRet = s.WaitFds(NULL);
		if (iRet > 0)
		{
			SOCKET_T fd;
			while((fd = s.NextReadFd()) != SOCKET_INVALID)
			{
				char buf[1024] = {0};
				DEBUG_LOG("ready fd : %d", fd);
				if(fd == client.GetSocket())
				{
					int len = client.Recv(buf, 1024);
					if (len > 0) {
						DEBUG_LOG("%s", buf);
					}
					else
					{
						ERROR_LOG("error code : %d", client.GetErrCode());
						ERROR_LOG("error msg : %s", client.GetErrMsg().c_str());
						s.DelReadFd(fd);
						return;
					}	
				}
				else if (fd == fileno(stdin))
				{
					char buf[1024] = {0};
					cin.getline(buf, 1024);
					int size = cin.gcount();
					
					if (size == 0)
					{
						ERROR_LOG("EOF");
						s.DelReadFd(fd);
						return;
					}
					
					int netsize = htonl(size);
					
					DEBUG_LOG("len = %d, Content = %s", size, buf);
					if (client.Send((char*)&netsize, sizeof(netsize)) <= 0)
					{
						DEBUG_LOG("errno : %d msg : %s", client.GetErrCode(), client.GetErrMsg().c_str());
					}

					if (client.Send(buf, size) <= 0)
					{
						DEBUG_LOG("errno : %d msg : %s", client.GetErrCode(), client.GetErrMsg().c_str());
					}
				}
			}
		}
		else if (iRet == 0)
		{
			ERROR_LOG("time out");
		}
		else
		{
			ERROR_LOG("WaitReadFd error");
			break;
		}
	}
}

void TestMsg()
{
	CMsg msg1("1", 1, "net");
	CMsg msg2("2", 2, "mod");
	msg1.TestPrint();
	msg2.TestPrint();
	std::string buf1;
	std::string buf2;
	msg1.Serialization(buf1);
	DEBUG_LOG("msg1 = %s", buf1.c_str());
	msg2.Serialization(buf2);
	DEBUG_LOG("msg2 = %s", buf2.c_str());
	CMsg* p = CreateMsg(0);
	if (p)
	{
		p->DeSerialization(buf2);
		p->TestPrint();
	}
	msg1.TestPrint();
	msg2.TestPrint();
}

void TestMmap(int flag = 1)
{
	CMmap map("lao.txt", 10);
	char* p = (char*)map.Open();
	int size = map.Size();
	if (p)
	{
		if (flag == 1)
		{
			p[size] = '\0';
			DEBUG_LOG("lao.txt : %s", p);
			char c;
			cin>>c;
			memset(p, c, size - 1);
			DEBUG_LOG("lao.txt : %s", p);
		}
		else
		{
			while(1)
			{
				sleep(1);
				DEBUG_LOG("lao.txt : %s", p);
			}
		}
	}
	else
	{
		ERROR_LOG("map.Open failed");
	}
	
}

void TestShareMem(int flag = 1)
{
	CShareMemory mem("panda.txt");
	
		if (flag == 1)
		{
			if (!mem.Create(100))
			{
				ERROR_LOG("mem.Create failed");
				return;
			}

			char* p = mem.Head();
			int size = mem.Size();
			DEBUG_LOG("size : %d", size);
			p[size] = '\0';
			DEBUG_LOG("mem : %s", p);
			char c;
			cin>>c;
			memset(p, c, size - 1);
			DEBUG_LOG("mem : %s", p);
		}
		else
		{
			if (!mem.Open(100))
			{
				ERROR_LOG("mem.Create failed");
				return;
			}
			
			char* p = mem.Head();
			int size = mem.Size();
			DEBUG_LOG("size : %d", size);
						
			while(1)
			{
				sleep(1);
				DEBUG_LOG("mem : %s", p);
			}
		}
	
}


void TestSem(int flag = 1)
{
	CSemaphore sem(1024);
	sem.Open();
	if (flag == 1)
	{
		sem.SetVal(0);
		while(1)
		{
			sem.V();
			DEBUG_LOG("Recv a signal");
		}
	}
	else
	{
		sem.P();
		DEBUG_LOG("Send a signal");
	}
}

void TestRandom()
{
	CRandom::SetSeed();
	for (int i = 0; i < 30; ++i)
	{
		DEBUG_LOG("%f", CRandom::Random0());
	}

	for (int i = 0; i < 30; ++i)
	{
		DEBUG_LOG("%d", CRandom::Random(30, 50));
	}

	for (int i = 0; i < 30; ++i)
	{
		DEBUG_LOG("%d", CRandom::Random(0, 1));
	}
}

int main(int argc, char **argv)
{
	CLog::GetInstance()->SetLogName("test");
	CLog::GetInstance()->SetLogPath("/opt/test/ctm/log");
	TestStringFunc();
	//TestRandom();
	//HandleSign();
	//TestGetHostByAddr();
	//TestGetHostByName();
	//TestSelect();
	//TestMsg();
	//TestMmap(S2I(argv[1]));
	//TestSem(S2I(argv[1]));
	//TestShareMem(S2I(argv[1]));
	//int a;
	//cin>>a;
	return 0;
}
