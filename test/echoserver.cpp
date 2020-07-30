
#include "net/tcpserver.h"
#include "testdef.h"

using namespace ctm;
using namespace std;

static const char endFlag[] = {'E', 'O', 'F' , '!', 0};

DECLARE_FUNC_EX(echoserver)
{
	CHECK_PARAM(argc, 3, "echo_server [ip] [port].");

	CCommonQueue recv;
	CTcpServer server(argv[1], atoi(argv[2]));
	server.SetOutMessageQueue(&recv);
	if (server.Init() == -1)
	{
		DEBUG_LOG("Init faild");
		return 0;
	}

	server.OnRunning();

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
			//fprintf(stderr, "conn fd:%d len:%d\n", netdata->m_conn.fd, netdata->m_buf->len);
			server.AsynSendData(netdata->m_conn, netdata->m_buf->data, netdata->m_buf->len);
		}
		else if (message->m_type == MSG_SYS_NET_CONN)
		{
			shared_ptr<CNetConnMessage> netconn = dynamic_pointer_cast<CNetConnMessage>(message);
			if (netconn->m_opt == CNetConnMessage::CONNECT_OK)
			{
				fprintf(stderr, "Client Conn ip:%s, port:%d, opt:%d\n", netconn->m_conn.ip.c_str(),
					  netconn->m_conn.port,
					  netconn->m_opt);
			}
		}

		recv.PopFront();
	}
	server.UnInit();

	return 0;
}

