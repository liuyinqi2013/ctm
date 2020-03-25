#include "common/singleton.h"
#include "common/string_tools.h"
#include "common/time_tools.h"
#include "common/msg.h"
#include "common/com.h"
#include "common/log.h"
#include "common/random.h"
#include "common/inifile.h"
#include "common/message.h"

#include "net/socket.h"
#include "net/select.h"
#include "ipc/mmap.h"
#include "ipc/semaphore.h"
#include "ipc/sharememory.h"
#include "module/timer.h"

#include <string.h>
#include <signal.h>


#include <iostream>
#include <ctype.h>
#include <iostream>

#include "thread/thread.h"


using namespace ctm;
using namespace std;

CCommonQueue queue1;

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
	void Hello() { cout << "hello" << endl; }
};

class TestThread : public CThread
{
public:
	TestThread() {}
	~TestThread() {}
protected:
	virtual int Run();
};

int TestThread::Run()
{
	while (1)
	{
		char a;
		cin >> a;
		queue1.PutMessage(shared_ptr<CMessage>(CreateMessage(MSG_SYS_TIMER)));
	}

	return 0;
}

void TestUnitThread()
{
	TestThread t;
	t.Start();
	if (t.Detach() != 0)
	{
		cout << "Detach failed" << endl;
	}

	cout << "Thread status " << t.GetStatus() << endl;
	sleep(10);
	t.Stop();
	cout << "Thread status " << t.GetStatus() << endl;
}

void TestStringFunc()
{
	string a("@@\r\n**}&\n@[@end@]");
	cout << a << endl;
	cout << EndsWith(a, "@@") << endl;
	cout << StartsWith(a, "@@") << endl;
	cout << Trimmed(a) << endl;
	std::vector<std::string> vecOutput;
	CutString(a, vecOutput, "[@end@]", false);
	cout << "item size = " << vecOutput.size() << endl;
	for (int i = 0; i < vecOutput.size(); ++i)
	{
		cout << "item = " << vecOutput[i] << endl;
	}

	cout << "-----------------BaseFileName----------------------" << endl;
	cout << BaseFileName("laod/wuda/laod.txt") << endl;
	cout << BaseFileName("/laod.txt") << endl;
	cout << BaseFileName("laod\\wuda\\laod.txt") << endl;
	cout << BaseFileName("laod/wuda/") << endl;
	cout << BaseFileName("wuda.txt") << endl;
	cout << "-----------------BaseFileName----------------------" << endl;

	cout << PathName("laod/wuda/laod.txt") << endl;
	cout << PathName("/laod.txt") << endl;
	cout << PathName("laod\\wuda\\laod.txt") << endl;
	cout << PathName("laod/wuda/") << endl;
	cout << PathName("wuda.txt") << endl;
}

void TestTime()
{
	CClock clock;
	cout << Timestamp() << endl;
	cout << MilliTimestamp() << endl;

	cout << DateTime() << endl;
	cout << DateTime(TDATE_FMT_1) << endl;
	cout << DateTime(TDATE_FMT_2) << endl;
	cout << DateTime(TDATE_FMT_3) << endl;
	cout << DateTime(TDATE_FMT_4) << endl;
	cout << DateTime(TDATE_FMT_5) << endl;

	cout << DateTime(TDATE_FMT_1, TTIME_FMT_0) << endl;
	cout << DateTime(TDATE_FMT_1, TTIME_FMT_1) << endl;
	cout << DateTime(TDATE_FMT_1, TTIME_FMT_2) << endl;
	cout << DateTime(TDATE_FMT_1, TTIME_FMT_3) << endl;

	cout << Date(TDATE_FMT_0) << endl;
	cout << Date(TDATE_FMT_1) << endl;
	cout << Date(TDATE_FMT_2) << endl;
	cout << Date(TDATE_FMT_3) << endl;
	cout << Date(TDATE_FMT_4) << endl;
	cout << Date(TDATE_FMT_5) << endl;

	cout << Time(TTIME_FMT_0) << endl;
	cout << Time(TTIME_FMT_1) << endl;
	cout << Time(TTIME_FMT_2) << endl;
	cout << Time(TTIME_FMT_3) << endl;

	cout << DayOfWeek() << endl;
	cout << WeekOfYear() << endl;

	cout << clock.RunInfo() << endl;
}

void TestTcpClient()
{
	TcpClient client;
	if (!client.Connect("127.0.0.1", 9999))
	{
		cout << "connect server failed!\n" << endl;
		return;
	}
	char buf[1024] = { 0 };
	int len = client.Recv(buf, 1024);
	if (len == -1)
	{
		cout << "recv failed!\n" << endl;
		return;
	}
	buf[len] = '\0';
	cout << "Recv : " << buf << endl;
	char* s = "hello server";
	client.Send(s, strlen(s));

}

void PrintHost(struct hostent* hostinfo)
{
	DEBUG_LOG("official name of host : %s", hostinfo->h_name);
	string hostAliases;
	char** head = hostinfo->h_aliases;
	for (; *head; ++head)
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

	char buf[128] = { 0 };
	string hostAddrs;
	head = hostinfo->h_addr_list;
	for (; *head; ++head)
	{
		inet_ntop(hostinfo->h_addrtype, *head, buf, sizeof(buf));
		hostAddrs += string(" ") + buf;
	}

	DEBUG_LOG("list of addresses  : %s", hostAddrs.c_str());

}

void TestGetHostByAddr()
{
	while (1)
	{
		DEBUG_LOG("Pealse input host ip:");
		char buf[128] = { 0 };
		cin.getline(buf, sizeof(buf));
		int size = cin.gcount();
		struct in_addr inaddr = { 0 };

		int ret = inet_pton(AF_INET, buf, &inaddr);
		if (ret <= 0)
		{
			DEBUG_LOG("Not in presentation format : %s", buf);
			continue;
		}

		struct hostent* hostinfo = gethostbyaddr((const char*)& inaddr, sizeof(inaddr), AF_INET);
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
	while (1)
	{
		DEBUG_LOG("Pealse input host name:");
		char buf[128] = { 0 };
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
	struct addrinfo addr = { 0 };
	addr.ai_family = AF_UNSPEC;
	addr.ai_socktype = SOCK_STREAM;
	addr.ai_flags = AI_NUMERICHOST;
	struct addrinfo* res, * p;
	if (0 != getaddrinfo("localhost", NULL, &addr, &res))
	{
		cout << "getaddrinfo failed" << endl;
	}

	for (p = res; p != NULL; p = p->ai_next)
	{
		char ipbuf[128] = { 0 };
		if (p->ai_family == AF_INET)
		{
			cout << "type AF_INET" << endl;
			struct sockaddr_in* sa = (struct sockaddr_in*)p->ai_addr;
			inet_ntop(AF_INET, &(sa->sin_addr), ipbuf, 128);
		}
		else if (p->ai_family == AF_INET6)
		{
			cout << "type AF_INET6" << endl;
			struct sockaddr_in* sa = (struct sockaddr_in*)p->ai_addr;
			inet_ntop(AF_INET6, &(sa->sin_addr), ipbuf, 128);
		}
		cout << "ip : " << ipbuf << endl;

		cout << "canonname : " << p->ai_canonname << endl;
	}
	freeaddrinfo(res);
}

void TestMsg()
{
	CMsg msg1("1", 1, "net");
	CMsg msg2("2", 2, "mod");
	msg1.TestPrint();
	msg2.TestPrint();
	DEBUG_LOG("msg1 = %s", msg1.ToString().c_str());
	DEBUG_LOG("msg2 = %s", msg2.ToString().c_str());
	CMsg* p = CreateMsg(0);
	if (p)
	{
		p->FromString(msg2.ToString());
		p->TestPrint();
	}
	msg1.TestPrint();
	msg2.TestPrint();
}

void TestMessage()
{
	TestThread t;
	t.Start();
	while(1)
	{
		cout << queue1.GetAndPopMessage()->ToJsonString() << endl;
	}
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
			cin >> c;
			memset(p, c, size - 1);
			DEBUG_LOG("lao.txt : %s", p);
		}
		else
		{
			while (1)
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
		cin >> c;
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

		while (1)
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
		while (1)
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
	for (int i = 0; i < 2000; ++i)
	{
		DEBUG_LOG("ddddddddddddddddddddddddddddddddddddddddddddd %f", CRandom::Random0_1());
	}

	for (int i = 0; i < 2000; ++i)
	{
		WARN_LOG("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa %d", CRandom::IntRandom(30, 50));
	}

	for (int i = 0; i < 2000; ++i)
	{
		ERROR_LOG("cccccccccccccccccccccccccccccccccccccccccccccc %f", CRandom::Random0_1());
	}

	for (int i = 0; i < 10000; ++i)
	{
		INFO_LOG("cccccccccccccccccccccccccccccccccccccccccccccc %u", CRandom::UIntRandom(0, 100));
	}

	for (int i = 0; i < 300; ++i)
	{
		INFO_LOG("xxxxxxxxxxxxxxxxxxxxxxxxxxxxx %f", CRandom::DoubleRandom(0.5, 10.0));
	}
}

struct list_node
{
	list_node(int _data, list_node* _next = NULL) :
	 data(_data), next(_next)
	{

	}
	int data;
	list_node* next;
};

void ShowList(list_node* head)
{
	while(head)
	{
		cout << head->data <<",";
		head = head->next;
	}
	cout<<endl;
}

list_node* ListReverse(list_node* head)
{
	list_node* p = head;
	list_node* q = NULL;
	list_node* tmp;
	while(p)
	{
		tmp = p->next;
		p->next = q;
		q = p;
		p = tmp;
	}

	head = q;

	return q;
}

void TestListReverse()
{
	list_node node10(10, NULL);
	list_node node9(9, &node10);
	list_node node8(8, &node9);
	list_node node7(7, &node8);
	list_node node6(6, &node7);
	list_node node5(5, &node6);
	list_node node4(4, &node5);
	list_node node3(3, &node4);
	list_node node2(2, &node3);
	list_node node1(1, &node2);

	list_node* head = &node1;
	
	ShowList(head);
	head = ListReverse(head);
	ShowList(head);
}

#pragma pack(2)
struct TestA
{
	int value;
	char type;
	short sShot;
	char bit;
};

class Foo
{
public:
	//virtual int func1() { return 1;}
	int  func2() { return 2;}
	int data1;
	static int data2;
};
 
int Foo::data2 = 1000;

int bitCount(int n)
{
	int count = 0;
	while(n)
	{
		++count;
		n = n & (n-1); 
	}
	return count;
}

int bitCount2(int n)
{
	static int index[16] = {0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4};
	int count = index[n & 0x000000FF] + index[(n>>4)& 0x000000FF] 
	+ index[(n>>8)& 0x000000FF] + index[(n>>12)& 0x000000FF];
	return count;
}

#define Show(expr) { cout << #expr << "=" << expr <<endl; }

void TestTencent()
{
	
	TestA* p = (TestA*)0x100000;
	cout << "sizeof(long long) = " << sizeof(long long) << endl; 
	cout << "sizeof(char) = " << sizeof(char) << endl; 
	cout << "sizeof(long) = " << sizeof(long) << endl; 
	cout << "sizeof(bool) = " << sizeof(bool) << endl; 
	cout << "sizeof(int) = " << sizeof(int) << endl; 
	cout << "sizeof(short) = " << sizeof(short) << endl; 
	cout<< "sizeof(TestA) = " <<sizeof(TestA) << endl;
	cout<< "sizeof(p) = " <<sizeof(p) << endl;

	long a = (long)((void*)p + 10);
	printf("a=%x\n", a);

	int n[] = {1, 2, 3, 4, 5};
	int* ptr = (int*)(&n+1);
	cout<< "*(n+1) = " <<*(n+1) << endl;
	cout<< "*(ptr - 1) = " <<*(ptr - 1) << endl;
	cout<< "sizeof(n) = " <<sizeof(n) << endl;

	Foo* foo = (Foo*)malloc(sizeof(Foo));
	Foo foo2;
	cout<< "sizeof(Foo) = " << sizeof(Foo)<< endl;
	cout<< "foo->func2() = " <<foo->func2() << endl;
	cout<< "foo->data1 = " <<foo->data1 << endl;
	cout<< "foo->data2 = " <<foo->data2 << endl;
	//cout<< "foo->func1() = " <<foo->func1() << endl;

	cout<< "foo2.func2() = " <<foo2.func2() << endl;
	cout<< "foo2.data1 = " <<foo2.data1 << endl;
	cout<< "foo2.data2 = " <<foo2.data2 << endl;
	//cout<< "foo2.func1() = " <<foo2.func1() << endl;

	cout<< "&foo2.data2" << (long)&foo2.data2<< endl;
	cout<< "&foo1->data2" << (long)&(foo->data2)<< endl;
	cout<< "&foo2.data1" << (long)&foo2.data1<< endl;
	cout<< "&foo1->data1" << (long)&(foo->data1)<< endl;

	Show(bitCount2(0));
	Show(bitCount2(1));
	Show(bitCount2(2));
	Show(bitCount2(3));
	Show(bitCount2(4));
	Show(bitCount2(5));
	Show(bitCount2(6));
	Show(bitCount2(7));
	Show(bitCount2(8));
	Show(bitCount2(9));
	Show(bitCount2(10));
	Show(bitCount2(11));
	Show(bitCount2(12));
	Show(bitCount2(13));
	Show(bitCount2(14));
	Show(bitCount2(15));
	int s = (1000<<2);
	Show(s);
}

void TestIni()
{
	CIniFile ini("conf.ini");
	ini.Load();

	string a;
	int b;
	double c;

	ini.Get("set_str", a);
	ini.Get("set_int", b);
	ini.Get("set_float", c);

	cout << a << endl;
	cout << b << endl;
	cout << c << endl;

	ini.Get("set", "set_str", a);
	ini.Get("set", "set_int", b);
	ini.Get("set", "set_float", c);

	cout << a << endl;
	cout << b << endl;
	cout << c << endl;

	//cout << ini["set_str"].AsString() << endl;
	//cout << ini["set_int"].AsInt() << endl;
	//cout << ini["set_float"].AsFloat() << endl;

	ini["set"]["set_str"] = "xxx";

	//cout << ini["set"]["set_str"].AsString() << endl;
	//cout << ini["set"]["set_int"].AsInt() << endl;
	//cout << ini["set"]["set_float"].AsFloat() << endl;

	//cout << ini.ToString()<<endl;

	ini.Save();
}

void TestTimer()
{
	CTimer timer;
	timer.Start();
	int timerId =  timer.AddTimer(100, 10, NULL, NULL, NULL);
	int timerId1 = timer.AddTimer(500, 10, NULL, NULL, NULL);
	int timerId2 = timer.AddTimer(1000, 10, NULL, NULL, NULL);
	DEBUG_LOG("timerId = %d, timerId1 = %d, timerId2 = %d,", timerId, timerId1, timerId2);
	sleep(10);
	timer.StopTimer(timerId1);
	timerId1 = timer.AddTimer(2000, 10, NULL, NULL, NULL);
	timer.StopAllTimer();
}

void WaitEnd()
{
	cout<<"Please enter any to exit!"<<endl;
	int a;
	cin >> a;
}

int main(int argc, char** argv)
{
	//CLog::GetInstance()->SetLogName("test");
	//CLog::GetInstance()->SetLogPath("logs/debug");
	CLog::GetInstance()->SetFileMaxSize(2);
	CLog::GetInstance()->SetLogLevel(CLog::LOG_DEBUG);
	//CLog::GetInstance()->SetLogPath("./");
	//TestStringFunc();
	//TestRandom();
	//HandleSign();
	//TestGetHostByAddr();
	//TestGetHostByName();
	//TestSelect();
	//TestMsg();
	//TestMmap(S2I(argv[1]));
	//TestSem(S2I(argv[1]));
	//TestShareMem(S2I(argv[1]));
	//TestIni();
	//TestUnitThread();
	//TestTime();
	//TestTimer();
	TestTencent();
	//TestMessage();
	//TestListReverse();

	//WaitEnd();

	return 0;
}
