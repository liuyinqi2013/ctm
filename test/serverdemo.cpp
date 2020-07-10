
#include "net/tcpserver.h"
#include "testdef.h"

using namespace ctm;
using namespace std;

DECLARE_FUNC_EX(tcpserver)
{
	CHECK_PARAM(argc, 3, "tcpserver [ip] [port].");

	CCommonQueue recv;
	CTcpServer server(argv[1], atoi(argv[2]));
	server.SetOutMessageQueue(&recv);
	if (server.Init() == -1)
	{
		DEBUG_LOG("Init faild");
		return 0;
	}

	FILE* fout = NULL;
	server.OnRunning();
	map<int, FILE*> connMap;
	string file("client_up");
	while(1)
	{
		shared_ptr<CMessage> message = recv.GetFront(10);
		if (message.get() == NULL)
		{
			usleep(1);
			continue;
		}

		if (message->m_type == MSG_SYS_NET_DATA)
		{
			shared_ptr<CNetDataMessage> netdata = dynamic_pointer_cast<CNetDataMessage>(message);
			fout = connMap[netdata->m_conn.fd];
			if (strncmp("&&END", netdata->m_buf->data, 5) == 0)
			{
				DEBUG_LOG("%d, Recv End!", netdata->m_conn.fd);
				fclose(fout);
				server.AsynSendData(netdata->m_conn, netdata->m_buf->data, netdata->m_buf->len);
				recv.PopFront();
				connMap.erase(netdata->m_conn.fd);
				continue;
			}

			fwrite(netdata->m_buf->data, 1, netdata->m_buf->len, fout);
			server.AsynSendData(netdata->m_conn, netdata->m_buf->data, netdata->m_buf->len);
		}
		else if (message->m_type == MSG_SYS_NET_CONN)
		{
			shared_ptr<CNetConnMessage> netconn = dynamic_pointer_cast<CNetConnMessage>(message);
			DEBUG_LOG("Client Conn ip : %s, port : %d, opt : %d", netconn->m_conn.ip.c_str(),
					  netconn->m_conn.port,
					  netconn->m_opt);
			if (netconn->m_opt == CNetConnMessage::CONNECT_OK)
			{
				string fullFileName = file + I2S(netconn->m_conn.fd);
				fout = fopen(fullFileName.c_str(), "wb");
				connMap[netconn->m_conn.fd] = fout;
			}
		}
		recv.PopFront();
	}
	server.UnInit();

	return 0;
}

