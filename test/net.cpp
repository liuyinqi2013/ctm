#include <stdio.h>
#include <string.h>
#include <map>
#include <unistd.h>
#include <pthread.h>
 #include <sys/stat.h>

#include <cassert>
#include <termios.h>
#include <sys/sysmacros.h>
#include "define.h"

using namespace std;

DECLARE_FUNC(echo_svr)
{
	int p = 6666;
	int fd = Listen(AF_INET, argv[1], p);
	if (fd < 0) {
		ERROR("listen failed. host:%s, port:%d", argv[1], p);
		return -1;
	}

	auto conn = Accept(fd);
	if (!conn.get()) {
		ERROR("accept failed. host:%s, port:%d", argv[1], p);
		return -1;
	}

	conn->Show();
	conn->ShutDown(1);

	while(1);

/*
	int n;
	Buffer buf(512);
	while(1) {
		n = conn->Read(&buf);
		if (n != 0) {
			ERROR("read failed. n:%d", buf.Len());
			return -1;
		}
		DEBUG("read len:%d, buf:%s", buf.Len(), buf.Data());

		n = conn->WriteFull(&buf);
		if (n != 0) {
			ERROR("write failed. n:%d", n);
			return -1;
		}

		DEBUG("buf freelen:%d, len:%d", buf.FreeLen(), buf.Len());
	}
*/

	return 0;
}

void OnConnCallBack(int code, const char* message, std::shared_ptr<TcpConn> conn) {
	if (code == -1 ) {
		ERROR("conn failed. code:%d, message:%s", code, message);
		return;
	}
	
	conn->Show();
	int n;
	char buf[512];
	
	while(1) {
		string tmp;
		cin>>tmp;
		if (tmp.length() == 0) {
			ERROR("end");
			return;
		}

		n = conn->Write((void*)tmp.data(), tmp.length());
		if (n <= 0) {
			ERROR("write failed. n:%d", n);
			return;
		}

		n = conn->Read(buf, 512);
		if (n <= 0) {
			ERROR("read failed. n:%d", n);
			return;
		}
		buf[n] = '\0';
		DEBUG("read len:%d, buf:%s", n, buf);
	}

	return;
}

DECLARE_FUNC(asyn_conn)
{
	CPoller p;
	p.Init();
	AsynTcpConnector connector(argv[1], atoi(argv[2]), OnConnCallBack, &p, 30);
	p.Run();
	return 0;
}

DECLARE_FUNC(echo_cli)
{
	auto conn = TcpConnect(argv[1], atoi(argv[2]));
	if (!conn.get()) {
		ERROR("accept failed. host:%s, port:%d", argv[1], argv[2]);
		return -1;
	}

	conn->Show();

	int n;
	char buf[512];
	
	while(1) {
		string tmp;
		cin>>tmp;
		if (tmp.length() == 0) {
			ERROR("end");
			return -1;
		}

		n = conn->Write((void*)tmp.data(), tmp.length());
		if (n <= 0) {
			ERROR("write failed. n:%d", n);
			return -1;
		}

		n = conn->Read(buf, 512);
		if (n <= 0) {
			ERROR("read failed. n:%d", n);
			return -1;
		}
		buf[n] = '\0';
		DEBUG("read len:%d, buf:%s", n, buf);
	}

	return 0;
}

DECLARE_FUNC(resolve)
{
	vector<CAddr> out;
	if (CResolve::LookupIPAddr(argv[1], out) < 0) {
		return -1;
	}

	for (int i = 0; i < out.size(); i++) {
		DEBUG("ip string:%s", out[i].String().c_str());
	}

	DEBUG("cname:%s", CResolve::LookupCNAME(argv[1]).c_str());

	return 0;
}

DECLARE_FUNC(canonname)
{
	struct addrinfo hints = { 0 };
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_CANONNAME;
	struct addrinfo* res, *p;

	if (getaddrinfo(argv[1], NULL, &hints, &res) < 0) {
		cout << "getaddrinfo failed" << endl;
	}

	for (p = res; p; p = p->ai_next)
	{
		cout << "canonname : " << p->ai_canonname << endl;
	}

	freeaddrinfo(res);
	return 0;
}

DECLARE_FUNC(hostip)
{
	printf("hostname:%s\n", HostName().c_str());
	std::vector<std::string> vecIps;
	GetHostIPs("www.sina.com", vecIps);
	for(size_t i = 0; i < vecIps.size(); ++i)
	{
		printf("ip%ld:%s\n", i + 1, vecIps[i].c_str());
	}
	return 0;
}