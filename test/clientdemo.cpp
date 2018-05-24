#include "common/singleton.h"
#include "common/string_tools.h"
#include "common/time_tools.h"
#include "common/msg.h"
#include "common/com.h"
#include "common/log.h"
#include "common/random.h"

#include "net/socket.h"
#include "net/select.h"
#include "net/netclient.h"

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

int main(int argc, char **argv)
{
	CLog::GetInstance()->SetLogName("test");
	CLog::GetInstance()->SetLogPath("/opt/test/ctm/log");

	CNetTcpClient client("127.0.0.1", 9999);

	client.SetEndFlag("[---@end@---]");
	if (!client.Init())
		return -1;

	client.Start();
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

