#include "testdef.h"

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
	TestThread(int s = 1) : status(s) {}
	~TestThread() {}
protected:
	int status;
	virtual int Run();
};

static int gvalue = 0x01020304;
int TestThread::Run()
{
	int i = 0;
	while (i < 100000)
	{
		if (status)
		{
			usleep(1);
			if (i % 2 == 0)
			{
				gvalue = 0x04030201;
			}
			else
			{
				gvalue = 0x01020304;
			}
		}
		else
		{
			usleep(1);
			printf("a = %0x\n", gvalue);
		}
		
		i++;
	}
	return 0;
}

DECLARE_FUNC(thread)
{
	TestThread t(1);
	TestThread t1(0);
	t.Start();
	t1.Start();
	t.Join();
	t1.Join();
	/*
	if (t.Detach() != 0)
	{
		cout << "Detach failed" << endl;
	}

	cout << "Thread status " << t.GetStatus() << endl;
	sleep(10);
	t.Stop();
	cout << "Thread status " << t.GetStatus() << endl;
	*/
	return 0;
}

DECLARE_FUNC(strtool)
{
	string a("@@\r\n**}&\n@[@end@]");
	cout << a << endl;
	cout << EndsWith(a, "@@") << endl;
	cout << StartsWith(a, "@@") << endl;
	cout << Trimmed(a) << endl;
	std::vector<std::string> vecOutput;
	CutString(a, vecOutput, "[@end@]", false);
	cout << "item size = " << vecOutput.size() << endl;
	for (size_t i = 0; i < vecOutput.size(); ++i)
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
	return 0;
}

DECLARE_FUNC_EX(timetool)
{
	// CClock clock;
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

	// cout << clock.RunInfo() << endl;
	return 0;
}

DECLARE_FUNC(addrinfo)
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
	return 0;
}

DECLARE_FUNC(hostip)
{
	printf("hostname:%s\n", LocalHostName().c_str());
	std::vector<std::string> vecIps;
	GetHostIps("www.baidu.com", vecIps);
	for(size_t i = 0; i < vecIps.size(); ++i)
	{
		printf("ip%d:%s\n", i + 1, vecIps[i].c_str());
	}
	return 0;
}

DECLARE_FUNC(msg)
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
	return 0;
}

DECLARE_FUNC(message)
{
	TestThread t;
	t.Start();
	while(1)
	{
		cout << queue1.GetFront()->ToJsonString() << endl;
	}
	return 0;
}

DECLARE_FUNC(mmap)
{
	CHECK_PARAM(argc, 2, "shamem [0|1].");
	int flag = atoi(argv[1]);
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

	return 0;
}

DECLARE_FUNC(shamem)
{
	CHECK_PARAM(argc, 2, "shamem [0|1].");
	int flag = atoi(argv[1]);
	CShareMemory mem("panda");
	if (flag == 1)
	{
		if (!mem.Open(100))
		{
			ERROR_LOG("mem.Create failed");
			return 0;
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
			return 0;
		}

		char* p = mem.Head();
		int size = mem.Size();
		DEBUG_LOG("size : %d", size);

		int i = 0;
		while (i++ < 100)
		{
			sleep(1);
			DEBUG_LOG("mem : %s", p);
		}
		mem.Destroy();
	}
	return 0;
}

DECLARE_FUNC(sem)
{
	CHECK_PARAM(argc, 2, "shamem [0|1].");
	int flag = atoi(argv[1]);
	CSemaphore sem(1024);
	sem.Open();
	if (flag == 1)
	{
		sem.SetVal(0);
		while (1)
		{
			sem.Wait();
			DEBUG_LOG("Recv a signal");
		}
	}
	else
	{
		sem.Post();
		DEBUG_LOG("Send a signal");
	}
	return 0;
}

DECLARE_FUNC(random)
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
	return 0;
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

DECLARE_FUNC(ListReverse)
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
	return 0;
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
	int func2() { return 2;}
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

DECLARE_FUNC(tencent)
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
	Show(sizeof(bool));
	int s = (1000<<2);
	Show(s);
	return 0;
}

DECLARE_FUNC(ini)
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
	return 0;
}

DECLARE_FUNC(timer)
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
	return 0;
}

int CompLen(const char* a, const char* b)
{
	int len = 0;
	while(a && b && *a != '\0' && *b != '\0' && *a++ == *b++) len++;
	return len;
}

int CompStr(const void *a, const void *b)
{
	return strcmp(*(const char**)a, *(const char**)b);
}

DECLARE_FUNC(maxsubstr)
{
	char a[2048] = {0};
	char* n[2048];
	char ch;
	int i = 0;
	cin >> ch;
	while(!cin.eof()) {
		a[i] = ch;
		n[i] = &a[i];
		++i;
		cin >> ch;
	}

	printf("i = %d, a=%s\n", i, a);

	int maxlen = 0;
	int tmp = 0;
	int index = 0;
	qsort(&n[0], i, sizeof(char*), CompStr);
	for (int j = 0; j < i - 1; j++)
	{
		//printf("n[%d]=%s\n", j, n[j]);
		tmp = CompLen(n[j], n[j+1]);
		if (tmp > maxlen) {
			maxlen = tmp;
			index = j;
		}
	}

	n[index][maxlen] = '\0';

	printf("\nlen=%d,s=[%s]\n", maxlen, n[index]);

	return 0;
}

class Panda
{
public:
	Panda(int val) : b(val)
	{

	}

	static void TestA()
	{
		printf("TestA\n");
	} 

	void TestB()
	{
		printf("TestB: %d\n", b);
	}

	static int a;
	int b;
	int c;
};

int Panda::a = 1;

typedef void (Panda::*Func)();

DECLARE_FUNC(object)
{
	Panda b(10), c(15);
	Panda* p = &b;
	Func f = &Panda::TestB;
	printf("Function A:%x\n", Panda::TestA);
	printf("Function B:%x\n", b.TestA);
	printf("Function B:%x\n", &Panda::TestB);
	printf("Function f:%x\n", f);
	printf("Function a:%x\n", Panda::a);
	printf("Function b:%x\n", &Panda::b);
	printf("Function c:%x\n", &Panda::c);
	printf("Function c:%x\n", &c.c);
	printf("Function c:%x\n", &b.c);

	b.TestB();
	(b.*f)();
	(c.*f)();
	c.TestA();
	return 0;
}

DECLARE_FUNC(md5)
{
	const char* data = "laodapanda";
	u_char buf[16] = {0};
	md5_t ctx;
	md5_init(&ctx);
	md5_update(&ctx, data, strlen(data));
	md5_final(buf, &ctx);
	for(int i = 0; i < 16; ++i)
	{
		printf("%02x", buf[i]);
	}

	printf("\n");
	printf("%s\n", EncodeHex((const char*)buf, 16).c_str());
	return 0;
}

DECLARE_FUNC(realpath)
{
	CHECK_PARAM(argc, 2, "realpath [filename].");
	char* realname = realpath(argv[1], NULL);
	printf("realname:%s\n", realname);
	free(realname);
	return 0;
}
