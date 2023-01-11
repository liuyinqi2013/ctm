#include <unistd.h>
#include <arpa/inet.h>
#include <algorithm>

#include <cassert>
#include <termios.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>

#include "define.h"

void DrawRectangle(int x, int y, int w, int h)
{
	printf("\e[%d;%dH+", y, x);
	for(int i = 1; i < w; i++)
	{
		printf("\e[%d;%dH-", y, x + i);
		printf("\e[%d;%dH-", y + h, x + i);
	}

	for(int i = 1; i < h; i++)
    {
        printf("\e[%d;%dH|", y + i, x);
        printf("\e[%d;%dH|", y + i, x + w);
    }
	

	printf("\e[%d;%dH+", y, x);
	printf("\e[%d;%dH+", y, x + w);
	printf("\e[%d;%dH+", y + h, x);
	printf("\e[%d;%dH+", y + h, x + w);		
}

void Move(int x, int y)
{
	printf("\e[%d;%dH", y, x);
}

DECLARE_FUNC(tcdemo)
{
	struct termios told;
	struct termios tnew;

	printf("■");
	tcgetattr(0, &told);
	tnew = told;

	tnew.c_lflag &= ~(ICANON | ECHO | ECHOE | IEXTEN);
	tnew.c_oflag &= ~ONLCR;
	tnew.c_cc[VMIN] = 1;
	tnew.c_cc[VTIME] = 0;
	
	
	tcsetattr(0, TCSANOW, &tnew);

	printf("\e[0;0H\e[2J");
	DrawRectangle(10, 10, 40, 20);

	Move(30, 20);

	fflush(stdout);
	int buf[2] = {0};
	while (buf[0] != 'q')
	{
		read(0, buf, 1);
		switch(buf[0])
		{
			case 'w':
				printf("\e[1A");
				break;
			case 's':
				printf("\e[1B");
				break;
			case 'a':
				printf("\e[1D");
				break;
			case 'd':
				printf("\e[1C");
				break;
			default:
				;
		}
		fflush(stdout);
	}


	for(int i = 0; i < 10 ; i++)
	{
		printf("\e[35m%-3d%c\e[0m\e[4D\e[?25l", (i + 1) * 100 /10, '%');
		fflush(stdout);
		sleep(1);
	}

	printf("\e[?25h\n");
		
	Move(0, 31);

	tcsetattr(0, TCSANOW, &told);

	return 0;
}

void ShowStat(struct stat& sb)
{
	printf("---------------------------------------------------\n");

	printf("ID of containing device:  [%lx,%lx]\n",
	     (long) major(sb.st_dev), (long) minor(sb.st_dev));

	printf("File type:                ");

	switch (sb.st_mode & S_IFMT) {
	case S_IFBLK:  printf("block device\n");            break;
	case S_IFCHR:  printf("character device\n");        break;
	case S_IFDIR:  printf("directory\n");               break;
	case S_IFIFO:  printf("FIFO/pipe\n");               break;
	case S_IFLNK:  printf("symlink\n");                 break;
	case S_IFREG:  printf("regular file\n");            break;
	case S_IFSOCK: printf("socket\n");                  break;
	default:       printf("unknown?\n");                break;
	}

	printf("I-node number:            %ld\n", (long) sb.st_ino);

	printf("Mode:                     %lo (octal)\n",
	        (unsigned long) sb.st_mode);

	printf("Link count:               %ld\n", (long) sb.st_nlink);
	printf("Ownership:                UID=%ld   GID=%ld\n",
	        (long) sb.st_uid, (long) sb.st_gid);

	printf("Preferred I/O block size: %ld bytes\n",
	        (long) sb.st_blksize);
	printf("File size:                %lld bytes\n",
	        (long long) sb.st_size);
	printf("Blocks allocated:         %lld\n",
	        (long long) sb.st_blocks);

	printf("Last status change:       %s", ctime(&sb.st_ctime));
	printf("Last file access:         %s", ctime(&sb.st_atime));
	printf("Last file modification:   %s", ctime(&sb.st_mtime));

	printf("---------------------------------------------------\n");
}

DECLARE_FUNC(iostat)
{
	struct stat sb;
	if (argc >= 2)
	{
		if (stat(argv[1], &sb)) 
		{
			printf("get stat faild:%s\n", argv[1]);
			return -1;
		}
	}
	else
	{
		if(fstat(STDOUT_FILENO, &sb) == -1) 
		{
			printf("get stat faild:STDOUT");
			return -1;
		}
	}

	ShowStat(sb);

	return 0;
}

DECLARE_FUNC(color)
{
	if (IsCharDevice(stdout))
	{
		printf("%s\n", ColorString("black", BLACK).c_str());
		printf("%s\n", ColorString("red", RED).c_str());
		printf("%s\n", ColorString("green", GREEN).c_str());
		printf("%s\n", ColorString("yellow", YELLOW).c_str());
		printf("%s\n", ColorString("bule", BLUE).c_str());
		printf("%s\n", ColorString("purple", PURPLE).c_str());
		printf("%s\n", ColorString("skybule", SKYBLUE).c_str());
		printf("%s\n", ColorString("white", WHITE).c_str());
	}
	else
	{
		printf("%s\n", "black");
		printf("%s\n", "red");
		printf("%s\n", "green");
		printf("%s\n", "yellow");
		printf("%s\n", "bule");
		printf("%s\n", "purple");
		printf("%s\n", "skybule");
		printf("%s\n", "white");
	}
	return 0;
}

int SumN(unsigned int n)
{
	float e = 0;
	for (unsigned int i = 1; i <= n; i++)
	{
		e += 1.0 / (1 + i);
	}

	return e;
}

DECLARE_FUNC(sum_e)
{
	cout << "e = " << SumN(100000) << endl;
	return 0;
}




CSafetyQueue<string> queue1;

void ShowSign(int sign)
{
	DEBUG("Recv a sign : %d", sign);
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

int TestThread::Run()
{
	string a;
	int cnt = 0;
	{
		CClock clock(m_strName);
		while(1)
		{
			int ret = queue1.GetPopFront(a, 1000);
			if (ret == queue1.ERR_TIME_OUT) {
				cout << m_strName << "GetPopFront time out" << endl;
				break;
			} else if(ret == queue1.ERR_OK) {
				cnt += 1;
			}
		}

		cout << m_strName <<" cnt : " << cnt << endl;
		cout << clock.RunInfo() << endl;
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

DECLARE_FUNC(addrinfo)
{
	struct addrinfo hints = { 0 };
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo* res, *p;

	if (getaddrinfo(argv[1], NULL, &hints, &res) < 0) {
		cout << "getaddrinfo failed" << endl;
	}

	for (p = res; p; p = p->ai_next)
	{
		char ipbuf[128] = { 0 };
		if (p->ai_family == AF_INET)
		{
			cout << "type AF_INET" << endl;
			struct sockaddr_in* sa = (struct sockaddr_in*)p->ai_addr;
			inet_ntop(AF_INET, &(sa->sin_addr), ipbuf, 128);
		} else if (p->ai_family == AF_INET6) {
			cout << "type AF_INET6" << endl;
			struct sockaddr_in* sa = (struct sockaddr_in*)p->ai_addr;
			inet_ntop(AF_INET6, &(sa->sin_addr), ipbuf, 128);
		}

		cout << "ip : " << ipbuf << endl;
		cout << "ai_next : " << p->ai_next << endl;
	}

	freeaddrinfo(res);
	return 0;
}

DECLARE_FUNC(message)
{
	TestThread t(1);
	TestThread t1(2);
	t.SetName("thread1");
	t1.SetName("thread2");
	t.Start();
	t1.Start();

	string a("main");
	for (int i = 0; i < 100000; i++)
	{
		int ret = queue1.PushBack(a + I2S(i));
		if (ret == queue1.ERR_TIME_OUT)
		{
			cout << "PushBack time out " << a + I2S(i) << endl;
		}
	}
	
	WaitEnd();
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
			DEBUG("lao.txt : %s", p);
			char c;
			cin >> c;
			memset(p, c, size - 1);
			DEBUG("lao.txt : %s", p);
		}
		else
		{
			while (1)
			{
				sleep(1);
				DEBUG("lao.txt : %s", p);
			}
		}
	}
	else
	{
		ERROR("map.Open failed");
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
			ERROR("mem.Create failed");
			return 0;
		}

		char* p = (char*)mem.Head();
		int size = mem.Size();
		DEBUG("size : %d", size);
		p[size] = '\0';
		DEBUG("mem : %s", p);
		char c;
		cin >> c;
		memset(p, c, size - 1);
		DEBUG("mem : %s", p);
	}
	else
	{
		if (!mem.Open(100))
		{
			ERROR("mem.Create failed");
			return 0;
		}

		char* p = (char*)mem.Head();
		int size = mem.Size();
		DEBUG("size : %d", size);

		int i = 0;
		while (i++ < 100)
		{
			sleep(1);
			DEBUG("mem : %s", p);
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
			DEBUG("Recv a signal");
		}
	}
	else
	{
		sem.Post();
		DEBUG("Send a signal");
	}
	return 0;
}

DECLARE_FUNC(random)
{
	for (int i = 0; i < 2000; ++i)
	{
		DEBUG("random0_1:%f", CRandom::Random0_1());
	}

	for (int i = 0; i < 2000; ++i)
	{
		WARN("int random:%d", CRandom::IntRandom(30, 50));
	}

	for (int i = 0; i < 10000; ++i)
	{
		INFO("uint random:%u", CRandom::UIntRandom(0, 100));
	}

	for (int i = 0; i < 300; ++i)
	{
		INFO("double random:%f", CRandom::DoubleRandom(0.5, 10.0));
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

DECLARE_FUNC(listreverse)
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
	char bit;
	short sShot;
};

class Foo
{
public:
	virtual int func1() { return 1;}
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

	long a = (long)((char*)p + 10);
	printf("a=%ld\n", a);

	int n[] = {1, 2, 3, 4, 5};
	int* ptr = (int*)(&n+1);
	cout<< "*(n+1) = " <<*(n+1) << endl;
	cout<< "*(ptr - 1) = " <<*(ptr - 1) << endl;
	cout<< "sizeof(n) = " <<sizeof(n) << endl;

	Foo* foo = new Foo;
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

	long p1 = 0;
	long p2 = 0;

	memcpy(&p1, foo, 8);
	memcpy(&p2, &foo2, 8);

	cout<< "&foo2 = " << (long)&foo2 << endl;
	cout<< "&foo1 = " << (long)foo << endl;
	cout<< "&foo2.data2 = " << (long)&foo2.data2<< endl;
	cout<< "&foo1->data2 = " << (long)&(foo->data2)<< endl;
	cout<< "&foo2.data1 = " << (long)&foo2.data1<< endl;
	cout<< "&foo1.data1 = " << (long)&(foo->data1)<< endl;

	cout<< "p1 = " << p1 << endl;
	cout<< "p2 = " << p2 << endl;

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
	TestTimer();
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

// return -1 no find pos
int GetComfortPos(char* chairList, int len)
{
	int pos = -1;

	int maxSpace = 0;
	int maxbeg = 0;
	int maxEnd = 0;

	for (int i = 0; i <= len; i++)
	{
		if (len == i || chairList[i])
		{
			if (i - pos > maxSpace)
			{
				maxSpace = i - pos;
				maxbeg = pos;
				maxEnd = i;
			}

			pos = i;
		}
	}

	if (maxSpace >= 2)
	{
		return (maxbeg + maxEnd) / 2;
	}

	return -1;
}

void Test()
{
	char a0[] = {1, 1, 1, 0, 0,  0, 0, 0, 0, 0}; // pos0 6
	char a1[] = {1, 1, 1, 1, 0,  0, 0, 0, 1, 1}; // pos0 5
	char a2[] = {1, 0, 0, 0, 1,  0, 0, 1, 1, 1}; // pos0 2
	char a3[] = {0, 0, 0, 0, 1,  1, 1, 1, 1, 1}; // pos0 1
	char a4[] = {0, 1, 1, 1, 1,  1, 1, 1, 1, 1}; // pos0 0
	char a5[] = {1, 1, 1, 1, 1,  1, 1, 1, 1, 0}; // pos0 9
	char a6[] = {1, 1, 1, 1, 1,  1, 1, 1, 1, 1}; // pos0 -1
	char a7[] = {1, 1, 1, 1, 0,  1, 1, 1, 1, 1}; // pos0 4

	char a8[]  = {1, 0, 1}; // pos0 1
	char a9[]  = {1, 0};    // pos0 1
	char a10[] = {0};       // pos0 0

	printf("pos0:%d\n", GetComfortPos(a0, sizeof(a0)));
	printf("pos1:%d\n", GetComfortPos(a1, sizeof(a1)));
	printf("pos2:%d\n", GetComfortPos(a2, sizeof(a2)));
	printf("pos3:%d\n", GetComfortPos(a3, sizeof(a3)));
	printf("pos4:%d\n", GetComfortPos(a4, sizeof(a4)));
	printf("pos5:%d\n", GetComfortPos(a5, sizeof(a5)));
	printf("pos6:%d\n", GetComfortPos(a6, sizeof(a6)));
	printf("pos7:%d\n", GetComfortPos(a7, sizeof(a7)));
	printf("pos8:%d\n", GetComfortPos(a8, sizeof(a8)));
	printf("pos9:%d\n", GetComfortPos(a9, sizeof(a9)));
	printf("pos9:%d\n", GetComfortPos(a10, sizeof(a10)));
}

DECLARE_FUNC(safepos)
{
	Test();
	return 0;
}

DECLARE_FUNC(lambda)
{
    int a[4] = { 1, 2, 3, 4 };
    int total = 0;
    for_each(a, a + 4, [&](int & x) { total += x; x *= 2; });
    cout << total << endl;  //输出 10
    for_each(a, a + 4, [=](int x) { cout << x << " "; });
	cout << endl; 

	for_each(a, a + 4, [&total](int& x) { total += x; x *= 2; });
	cout << total << endl;  //输出 30

    return 0;
}