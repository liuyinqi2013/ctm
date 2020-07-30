#include <string.h>
#include <signal.h>

#include <iostream>
#include <ctype.h>
#include <iostream>

#include "testdef.h"
#include "net/netclient.h"
#include "net/tcpclient.h"
#include "net/tcpserver.h"

using namespace ctm;
using namespace std;

static const char endFlag[] = {'E', 'O', 'F' , '!', 0};

void Test(int argc, char **argv)
{
	CNetTcpClient client("127.0.0.1", 9999);

	client.SetEndFlag("[---@end@---]");
	if (!client.Init())
		return ;

	client.Start();
	FILE* fin = NULL;
	if (argc > 1)
	{
		fin = fopen(argv[1], "rb+");
		string filePathName(argv[1]);
		string fileName = filePathName;
		size_t pos = filePathName.rfind("/");
		if (pos != string::npos)
		{
			fileName = filePathName.substr(pos + 1);
		}
		string strHead = string("[--filename--]:") + fileName;
		DEBUG_LOG("strHead:%s", strHead.c_str());
		client.SendNetPack(strHead);
	}
	else
	{
		fin = stdin;
		ERROR_LOG("stdin fileno : %d", fileno(fin));
	}

	while (1)
	{
		DEBUG_LOG("%s", client.GetNetPack().c_str());
		char buf[1024] = {0};
		fgets(buf, 1024, fin);

		DEBUG_LOG("send buf = %s, size = %d", buf, strlen(buf));
		if (client.SendNetPack(string(buf, strlen(buf))) <= 0)
		{
			DEBUG_LOG("SendNetPack errno");
		}
	}
}

void EchoClient(int argc, char **argv)
{
	CCommonQueue recv;
	CTcpClient client(argv[1], atoi(argv[2]));
	client.SetOutMessageQueue(&recv);
	if (client.Init() == -1)
	{
		fprintf(stderr, "Init faild\n");
		return ;
	}

	char buf[4096] = {0};
	int len = 0;
	int sendTotalLen = 0;
	int recvTotalLen = 0;

	FILE* fin  = stdin;
	FILE* fout = stdout;
	client.OnRunning();
	bool isConn = false;
	bool isInputEnd = false;
	CClock clock;
	while(1)
	{
		shared_ptr<CMessage> message = recv.NonBlockGetFront();
		if(message.get() != NULL)
		{
			if (message->m_type == MSG_SYS_NET_DATA)
			{
				shared_ptr<CNetDataMessage> netdata = dynamic_pointer_cast<CNetDataMessage>(message);
				if (strncmp(endFlag, netdata->m_buf->data, strlen(endFlag)) == 0)
				{
					break;
				}

				recvTotalLen += netdata->m_buf->len;
				fwrite(netdata->m_buf->data, 1, netdata->m_buf->len, fout);
			}
			else if (message->m_type == MSG_SYS_NET_CONN)
			{
				shared_ptr<CNetConnMessage> netconn = dynamic_pointer_cast<CNetConnMessage>(message);
				if (netconn->m_opt == CNetConnMessage::CONNECT_OK)
				{
					isConn = true;
				}
				else
				{
					fprintf(stderr, "Connect failed!\n");
					break;
				}
			}
			recv.PopFront();
		}
		
		if (isConn && !isInputEnd)
		{
			len = fread(buf, 1, 4096, fin);

			if (len > 0)
			{
				sendTotalLen += len;
				client.SyncSendData(buf, len);
			}

			if (feof(fin))
			{
				client.SyncSendData(endFlag, strlen(endFlag));
				isInputEnd = true;
			}
		}
		else
		{
			usleep(1);
		}
	}

	client.UnInit();
	fprintf(stderr, "send len %d\n", sendTotalLen);
	fprintf(stderr, "recv len %d\n", recvTotalLen);
	fprintf(stderr, "%s\n", clock.RunInfo().c_str());
}

DECLARE_FUNC_EX(echoclient)
{
	CHECK_PARAM(argc, 3, "echo_client [ip] [port].");

	EchoClient(argc, argv);
	
	return 0;
}

