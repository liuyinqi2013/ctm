#include <stdio.h>
#include <string.h>
#include <map>
#include <unistd.h>
#include <pthread.h>
 #include <sys/stat.h>

#include <cassert>
#include <termios.h>
#include <sys/sysmacros.h>
#include "testdef.h"

using namespace std;

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
	//printf("\e[%d;%dH", 20, 10);
	
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
				//printf("%c", buf[0]);
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

DECLARE_FUNC(chartable)
{
	CCharTable tab(4, 5);
	CStyle* style = tab.CreateStyle();

	style->SetHorAlign(CStyle::RIGHT);
	style->SetVerAlign(CStyle::VCENTER);
	style->SetColor(CStyle::RED);

	tab.Write(0, 0, "id");
	tab.Write(0, 1, "name");
	tab.Write(0, 2, "age");
	tab.Write(0, 3, "sexy");
	tab.Write(0, 4, "addr");

	tab.Column(0)->SetStyle(style);
	tab.Column(4)->SetStyle(style);
	tab.Column(4)->SetWidth(10);

	for (int i = 1; i < 4; i++)
	{
		tab.Row(i)->SetHight(3);
		tab.Write(i, 0, I2S(i+1));
		tab.Write(i, 1, "panda");
		tab.Write(i, 2, "10");
		tab.Write(i, 3, "man");
		tab.Write(i, 4, "sheng zheng baoan xi xiang");
	}

	tab.Print();
	//cout << tab.TopLine() << endl;
	//cout << StrNum('*', 10) << endl;
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

DECLARE_FUNC(testqueue)
{
	CSafetyQueue<int> queue;
	queue.PushBack(1);
	queue.PushBack(2);

	if (CSafetyQueue<int>::ERR_TIME_OUT == queue.PushBack(3, 500))
	{
		printf("PushBack time out\n");
	}

	if (CSafetyQueue<int>::ERR_TIME_OUT == queue.PushFront(4, 500))
	{
		printf("PushFront time out\n");
	}

	int a = 0;

	while (1)
	{
		int ret = queue.GetPopBack(a, 500);
		if (CSafetyQueue<int>::ERR_TIME_OUT == ret)
		{
			printf("GetPopBack time out\n");
		}
		else if (ret == -1)
		{
			printf("error !\n");
			break;
		}
		else
		{
			printf("a = %d\n", a);
		}
		
	}
	return 0;
}

DECLARE_FUNC(echo_ser)
{
	CHECK_PARAM(argc, 3, "echo_ser [ip] [port].");
	CLog log("echo_ser");
	CEchoServer echoServ;
	if (echoServ.Init(argv[1], atoi(argv[2]), &log) == -1)
	{
		fprintf(stderr, "echo server init failed\n");
		return -1;
	}

	CTM_INFO_LOG(&log, "echo_ser init OK");

	while(echoServ.Execute() == 0);

	return 0;
}

DECLARE_FUNC(echo_cli)
{
	CHECK_PARAM(argc, 3, "echo_cli [ip] [port].");

	CLog log("echo_cli");
	CEchoClient echoCli;
	CClock clock;
	if (echoCli.Init(argv[1], atoi(argv[2]), &log) == -1)
	{
		fprintf(stderr, "echo client init failed\n");
		return -1;
	}

	/*
	sigset_t set;
	sigemptyset(&set);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGPIPE);
	sigaddset(&set, SIGSEGV);
	sigaddset(&set, SIGFPE);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
	*/

	CTM_INFO_LOG(&log, "echo_cli init OK");

	while(echoCli.Execute() == 0);

	CTM_INFO_LOG(&log, "Send Over len = %d", echoCli.m_sendLen);
	CTM_INFO_LOG(&log, "Recv Over len = %d", echoCli.m_recvLen);
	CTM_INFO_LOG(&log, "%s", clock.RunInfo().c_str());

	return 0;
}

DECLARE_FUNC(test_ser)
{
	CClock clock;
	CTestServer testSer;
	if (testSer.Init(CLog::GetInstance()) == -1)
	{
		fprintf(stderr, "test server init failed\n");
		return -1;
	}

	CTM_INFO_LOG(CLog::GetInstance(), "test_ser init OK");

	while(testSer.Execute() == 0);

	CTM_INFO_LOG(CLog::GetInstance(), "%s", clock.RunInfo().c_str());

	return 0;
}

DECLARE_FUNC(base_game_ser)
{
	CClock clock;
	CBaseGame gameServer;

	printf("gameServer %x\n", &gameServer);

	if (gameServer.Init(CLog::GetInstance()) == -1)
	{
		CTM_ERROR_LOG(CLog::GetInstance(), "base_game init failed\n");
		return -1;
	}

	gameServer.StartListen(9999);
	gameServer.StartListen(8888);

	gameServer.MemroyConn(2048, 2048, true);
	gameServer.MemroyConn(2049, 2048, true);

	CTM_INFO_LOG(CLog::GetInstance(), "gameServer init OK");

	gameServer.SetHeartBeat(true);
	gameServer.SetHBInterval(5);

	gameServer.LoopRun();

	CTM_INFO_LOG(CLog::GetInstance(), "%s", clock.RunInfo().c_str());

	return 0;
}

DECLARE_FUNC(base_game_cli)
{
	CClock clock;
	CBaseGame gameClient;

	printf("gameClient %x\n", &gameClient);

	if (gameClient.Init(CLog::GetInstance()) == -1)
	{
		CTM_ERROR_LOG(CLog::GetInstance(), "base_game init failed\n");
		return -1;
	}

	CTM_INFO_LOG(CLog::GetInstance(), "gameClient init OK");

	CConn* conn  = gameClient.Connect("127.0.0.1", 9999);
	CConn* conn1 = gameClient.Connect("127.0.0.1", 8888);

	// CConn* conn2  = gameClient.MemroyConn(2048, 2048, false);
	// CConn* conn3 = gameClient.MemroyConn(2049, 2048, false);

	gameClient.StartTimer(500, 50, (TimerCallBack)&CBaseGame::TestEchoTimer, conn, (void*)"hello 9999");
	gameClient.StartTimer(100, 20, (TimerCallBack)&CBaseGame::TestEchoTimer, conn, (void*)"hello 8888");

	// gameClient.SetHeartBeat(true);
	// gameClient.SetHBInterval(5);

	gameClient.LoopRun();

	CTM_INFO_LOG(CLog::GetInstance(), "%s", clock.RunInfo().c_str());

	return 0;
}

DECLARE_FUNC(mem_queue_send)
{
	CClock clock;

	CSemaphore sem(1024);
	sem.Open();
	sem.SetVal(1);

	CShareMemory mem(1024);

	mem.Open(32 * 1024);

	CTM_INFO_LOG(CLog::GetInstance(), "head = %x size = %d", mem.Head(), mem.Size());

	CMemoryQueue queue(mem.Head(), mem.Size(), sem.GetSemId(), 0);

	char data[]  = "hello world";
	char data1[] = "I am liu yin qi hello world";

	char* d = data;
	int len = 0;

	int i = 0;
	int ret = 0;

	while(1)
	{
		if ((i % 2) == 0)
		{
			d = data;
			len = sizeof(data);
		}
		else
		{
			d = data1;
			len = sizeof(data1);
		}

		CTM_INFO_LOG(CLog::GetInstance(), "cnt = %d", queue.Count());

		ret = queue.Push(d, len);
		if (ret == 0)
		{
			CTM_INFO_LOG(CLog::GetInstance(), "Send failed");
		}

		++i;
		
		sleep(1);
	}

	CTM_INFO_LOG(CLog::GetInstance(), "%s", clock.RunInfo().c_str());

	return 0;
}

DECLARE_FUNC(mem_queue_recv)
{
	CClock clock;

	CSemaphore sem(1024);
	sem.Open();

	CShareMemory mem(1024);

	mem.Open(32 * 1024);

	CMemoryQueue queue(mem.Head(), mem.Size(), sem.GetSemId(), 0);

	char data[64] = {0};
	uint32_t len  = 64;

	int ret = 0;

	while(1)
	{
		CTM_INFO_LOG(CLog::GetInstance(), "cnt = %d", queue.Count());

		ret = queue.Get(data, len);

		if (ret == 0)
		{
			CTM_INFO_LOG(CLog::GetInstance(), "Get failed");
		}
		else
		{
			data[ret] = 0;
			CTM_INFO_LOG(CLog::GetInstance(), "ret=%d, %s", ret, data);
		}

		sleep(1);
	}

	CTM_INFO_LOG(CLog::GetInstance(), "%s", clock.RunInfo().c_str());

	return 0;
}