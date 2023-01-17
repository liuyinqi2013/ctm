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

class EchoClient : public TcpConnHandler, public CFile::CHandler
{
public :
	EchoClient(CPoller* poller) : m_conn(NULL), m_stdin(new CFile(0, poller)), m_buf(512)
	{
		m_stdin->SetHandler(this);
	}

	virtual void OnConnect(int code, const char* message, TcpConn* conn) 
	{
		if (code) {
			TcpConnHandler::OnConnect(code, message, conn);
			return;
		}
		m_conn = conn;
		conn->Show();
		conn->SetHandler(this);
		conn->SetEvent(EvRead);
		m_stdin->SetEvent(EvRead);
	}

	virtual void OnRead(CFile* file) 
	{
		int ret;
		if (file == m_stdin) {
			ret = m_stdin->Read(&m_buf);
			if (ret != IO_OK) {
				ERROR("read stdin failed. ret:%d, len:%d", ret, m_buf.Len());
				exit(0);
			}

			m_conn->WriteFull(&m_buf);
			if (ret != IO_OK) {
				ERROR("write conn failed. ret:%d", ret);
				exit(0);
			}

		} else {
			ret = m_conn->Read(&m_buf);
			if (ret != IO_OK) {
				ERROR("read conn failed. ret:%d", ret);
				exit(0);
			}

			m_stdin->WriteFull(&m_buf);
			if (ret != IO_OK) {
				ERROR("write stdin failed. ret:%d", ret);
				exit(0);
			}
		}
	}
private:
	TcpConn* m_conn;
	CFile *m_stdin;
	Buffer m_buf;
};

class EchoServer : public TcpConnHandler, public CFile::CHandler
{
public :
	EchoServer(CPoller* poller) : m_buf(512)
	{
	}

	virtual void OnConnect(int code, const char* message, TcpConn* conn) 
	{
		if (code) {
			TcpConnHandler::OnConnect(code, message, conn);
			return;
		}
		conn->Show();
		conn->SetHandler(this);
		conn->SetEvent(EvRead);
		m_conns[conn->GetFd()] = conn;
	}

	virtual void OnRead(CFile* file) 
	{
		auto conn = reinterpret_cast<TcpConn*>(file);
		int ret = conn->Read(&m_buf);
		if (ret != IO_OK) {
			Remove(conn);
			ERROR("read failed. conn:%s, ret:%d", conn->GetRemoteAddr().c_str(), ret);
			return;
		}

		conn->WriteFull(&m_buf);
		if (ret != IO_OK) {
			Remove(conn);
			ERROR("write failed. conn:%s. ret:%d", conn->GetRemoteAddr().c_str(), ret);
			return;
		}
	}

	virtual void OnError(CFile* file)
	{
		DEBUG("on error. fd:%d", file->GetFd());
		Remove(reinterpret_cast<TcpConn*>(file));
	}

	void Remove(TcpConn* conn)
	{
		m_conns.erase(conn->GetFd());
		DELETE(conn);
	}
private:
	Buffer m_buf;
	unordered_map<int, TcpConn*> m_conns;
};


DECLARE_FUNC(echo_svr)
{
	CPoller p;
	int ret = p.Init();
	if (ret) {
		ERROR("poller init failed.");
		return -1;
	}

	EchoServer server(&p);
	TcpListener listener(&p, &server, CAddr(argv[1], atoi(argv[2])));
	listener.Listen();
	p.Run();

	return 0;
}

DECLARE_FUNC(echo_cli)
{
	CPoller p;
	int ret = p.Init();
	if (ret) {
		ERROR("poller init failed.");
		return -1;
	}

	EchoClient handler(&p);
	TcpConnector connector(&p, &handler, argv[1], atoi(argv[2]), 30);
	p.Run();

	return 0;
}

DECLARE_FUNC(asyn_conn)
{
	CPoller p;
	int ret = p.Init();
	if (ret) {
		ERROR("poller init failed.");
		return -1;
	}

	TcpConnHandler handler;
	TcpConnector connector(&p, &handler, argv[1], atoi(argv[2]), 30);
	p.Run();
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