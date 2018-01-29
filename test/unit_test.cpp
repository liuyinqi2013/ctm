#include "common/singleton.h"
#include "common/string_tools.h"
#include "common/time_tools.h"
#include "common/msg.h"

#include "net/socket.h"
#include "net/select.h"

#include <string.h>

#include <iostream>
#include <ctype.h>

using namespace ctm;
using namespace std;

class TestSingleton : public CSingleton<TestSingleton>
{
public:
	void Hello() { cout<<"hello"<<endl; }
};

void TestStringFunc()
{
	string a("\r\n**}&\n");
	cout<<Trimmed(a)<<endl;
	cout<<Sec()<<endl;
	cout<<Usec()<<endl;
	cout<<DateTime()<<endl;
	cout<<DateTime(TFMT_1)<<endl;
	cout<<DateTime(TFMT_2)<<endl;
	TestSingleton::GetInstance()->Hello();

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

	char* send = "hello server";
	client.Send(send, strlen(send));
	
	CSelect s;

	s.AddReadFd(client.GetSocket());

	while (1)
	{
		struct timeval timeOut = { 5, 10 };

		int iRet = s.WaitFds(NULL);
		if (iRet > 0)
		{
			SOCKET_T fd;
			while((fd = s.NextReadFd()) != SOCKET_INVALID)
			{
				DEBUG_LOG("ready fd : %d", fd);
				if(fd == client.GetSocket())
				{
					char buf[128] = {0};
					int len = client.Recv(buf, 128);
					if (len > 0) {
						DEBUG_LOG("read : %s", buf);
					}
					else {
						ERROR_LOG("error code : %d", client.GetErrCode());
						ERROR_LOG("error msg : %s", client.GetErrMsg().c_str());
						s.DelReadFd(fd);
						continue;
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
	CMsg msg1(1,"net");
	CMsg msg2(2,"mod");
	msg1.TestPrint();
	msg2.TestPrint();
	std::string buf;
	msg1.Serialization(buf);
	msg2.DeSerialization(buf);
	CMsg* p = CreateMsg(0);
	if (p)
	{
		p->DeSerialization(buf);
		p->TestPrint();
	}
	msg1.TestPrint();
	msg2.TestPrint();
}

int main(int argc, char **argv)
{
	TestSelect();
	//TestMsg();
	int a;
	cin>>a;
	return 0;
}
