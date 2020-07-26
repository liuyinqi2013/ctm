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
		DEBUG_LOG("Init faild");
		return ;
	}

	char buf[4096] = {0};
	int len = 0;
	int sendTotalLen = 0;
	int recvTotalLen = 0;

	FILE* fin  = stdin;
	FILE* fout = fopen("echo_return", "wb");
	client.OnRunning();
	bool isConn = false;
	CClock clock;
	while(1)
	{
		shared_ptr<CMessage> message = recv.NonBlockGetFront();
		if(message.get() != NULL)
		{
			if (message->m_type == MSG_SYS_NET_DATA)
			{
				shared_ptr<CNetDataMessage> netdata = dynamic_pointer_cast<CNetDataMessage>(message);
				recvTotalLen += netdata->m_buf->len;
				fwrite(netdata->m_buf->data, 1, netdata->m_buf->len, fout);
			}
			else if (message->m_type == MSG_SYS_NET_CONN)
			{
				shared_ptr<CNetConnMessage> netconn = dynamic_pointer_cast<CNetConnMessage>(message);
				DEBUG_LOG("Conn ip : %s, port : %d, opt : %d", netconn->m_conn.ip.c_str(),
						  netconn->m_conn.port,
						  netconn->m_opt);

				if (netconn->m_opt == CNetConnMessage::CONNECT_OK)
				{
					isConn = true;
				}
				else
				{
					break;
				}
			}
			recv.PopFront();
		}
		
		if (isConn)
		{
			len = fread(buf, 1, 4096, fin);

			if (len > 0)
			{
				sendTotalLen += len;
				client.SyncSendData(buf, len);
			}

			if (feof(fin))
			{
				client.WriteClose();
			}
		}
		else
		{
			usleep(1);
		}
	}

	fclose(fout);
	client.UnInit();
	DEBUG_LOG("send len %d",  sendTotalLen);
	DEBUG_LOG("recv len %d", recvTotalLen);
	DEBUG_LOG("%s", clock.RunInfo().c_str());
}

DECLARE_FUNC_EX(echo_client)
{
	CHECK_PARAM(argc, 3, "echo_client [ip] [port].");

	EchoClient(argc, argv);
	//CLog::GetInstance()->SetLogName("testclient");
	//CLog::GetInstance()->SetLogPath("./log/");

	/*
	TcpClient client;

	if (!client.Connect("127.0.0.1", 9999))
	{
		DEBUG_LOG("connect server failed : %d, %s!", client.GetErrCode(),  client.GetErrMsg().c_str());
		return -1;
	}
 
	
	CSelect s;

	FILE* fin = NULL;
	if (argc > 1)
	{
		fin = fopen(argv[1], "rb+");
		string filePathName(argv[1]);
		string fileName = filePathName;
		int pos = filePathName.rfind("/");
		if (pos != string::npos)
		{
			fileName = filePathName.substr(pos + 1);
		}
		string strHead = string("[--filename--]:") + fileName + "[---@end@---]";
		DEBUG_LOG("strHead:%s", strHead.c_str());
		client.Send(strHead.data(), strHead.size());
	}
	else
	{
		fin = stdin;
		ERROR_LOG("stdin fileno : %d", fileno(fin));
	}

	unsigned long total_size = 0;
	s.AddReadFd(client.GetSocket());
	s.AddReadFd(fileno(fin));
	unsigned long long beginTime = UTime();
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
				//DEBUG_LOG("ready fd : %d", fd);
				if(fd == client.GetSocket())
				{
					int len = client.Recv(buf, 1024);
					if (len > 0) {
						if (fin == stdin)
						{
							DEBUG_LOG("%s", buf);
						}
					}
					else
					{
						ERROR_LOG("error code : %d", client.GetErrCode());
						ERROR_LOG("error msg : %s", client.GetErrMsg().c_str());
						s.DelReadFd(fd);
						client.m_tcpSock.ShutDown(SHUT_RD);
						goto quit;
					}	
				}
				else if (fd == fileno(fin) && fd != 0)
				{
					char buf[4096 + 32] = {0};
					int len = fread(buf, sizeof(char), 4096, fin);
					total_size += len;
					strncpy(buf + len, "[---@end@---]", strlen("[---@end@---]"));
					int size = len + strlen("[---@end@---]");
					//DEBUG_LOG("send buf = %s, len = %d", buf, size);
					if (client.Send(buf, size) <= 0)
					{
						DEBUG_LOG("errno : %d msg : %s", client.GetErrCode(), client.GetErrMsg().c_str());
					}

					if (feof(fin))
					{
						fclose(fin);
						DEBUG_LOG("[Send End]");
						string strEnd = string("[--end--]") + "[---@end@---]";
						client.Send(strEnd.data(), strEnd.size());
						client.m_tcpSock.ShutDown(SHUT_WR);
						s.DelReadFd(fd);
						DEBUG_LOG("file total size =%ul", total_size);
					}
				}
				else if (fd == 0)
				{
					char buf[4096 + 32] = {0};
					fgets(buf, 4096, fin);
					int len = strlen(buf);
					strncpy(buf + len, "[---@end@---]", strlen("[---@end@---]"));
					int size = len + strlen("[---@end@---]");
					DEBUG_LOG("send buf = %s, size = %d", buf, size);
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


quit:
	unsigned long long endTime = UTime();
	DEBUG_LOG("begin time = %ull us", beginTime);
	DEBUG_LOG("end time = %ull us", endTime);
	DEBUG_LOG("used time = %ull us", endTime - beginTime);
	*/
	
	return 0;
}

