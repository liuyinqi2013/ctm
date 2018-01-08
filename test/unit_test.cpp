#include "common/singleton.h"
#include "common/string_tools.h"
#include "common/time_tools.h"
#include "net/socket.h"
#include <string.h>

#include <iostream>
#include <ctype.h>

using namespace ctm;
using namespace std;

class TestSingleton : public Singleton<TestSingleton>
{
public:
	void Hello() { cout<<"hello"<<endl; }
};

int main(int argc, char **argv)
{
	//string a("\r\n**}&\n");
	//cout<<Trimmed(a)<<endl;
	//cout<<Sec()<<endl;
	//cout<<Usec()<<endl;
	//cout<<DateTime()<<endl;
	//cout<<DateTime(TFMT_1)<<endl;
	//cout<<DateTime(TFMT_2)<<endl;
	//TestSingleton::GetInstance()->Hello();

	TcpClient client;
	
	if (!client.Connect("127.0.0.1", 9999))
	{
		cout<<"connect server failed!\n"<<endl;
		return -1;
	}
	char buf[1024] = {0};
	int len = client.Recv(buf, 1024);
	if (len == -1)
	{
		cout<<"recv failed!\n"<<endl;
		return -1;
	}
	buf[len] = '\0';
	cout<<"Recv : "<<buf<<endl;
	char* s = "hello server";
	client.Send(s, strlen(s));
	int a;
	cin>>a;

	/*
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
	*/

	return 0;
}
