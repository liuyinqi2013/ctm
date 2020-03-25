#include "common/singleton.h"
#include "common/string_tools.h"
#include "common/time_tools.h"
#include "common/msg.h"
#include "common/com.h"
#include "common/log.h"
#include "common/random.h"

#include "net/socket.h"
#include "net/select.h"
#include "ipc/mmap.h"
#include "ipc/semaphore.h"
#include "ipc/sharememory.h"
#include "net/tcpserver.h"

#include <string.h>
#include <signal.h>


#include <iostream>
#include <ctype.h>
#include <iostream>

#include "tcpserver.h"

using namespace ctm;
using namespace std;

int main(int argc, char **argv)
{
	CCommonQueue recv;
	CTcpServer server("127.0.0.1", 9999);
	server.SetOutMessageQueue(&recv);
	if (server.Init() == -1)
	{
		DEBUG_LOG("Init faild");
		return 0;
	}

	FILE* fout = fopen("client_up", "wb");
	server.OnRunning();
	while(1)
	{
		shared_ptr<CMessage> message = recv.GetAndPopMessage();

		if (message->m_type == MSG_SYS_NET_DATA)
		{
			shared_ptr<CNetDataMessage> netdata = dynamic_pointer_cast<CNetDataMessage>(message);
			DEBUG_LOG("Recv Data len = %d" , netdata->m_buf->len);
			if (strncmp("&&END", netdata->m_buf->data, 5) == 0)
			{
				DEBUG_LOG("Recv End!");
				fclose(fout);
				server.SendData(netdata->m_conn, netdata->m_buf->data, netdata->m_buf->len);
				continue;
			}

			fwrite(netdata->m_buf->data, 1, netdata->m_buf->len, fout);
			server.SendData(netdata->m_conn, netdata->m_buf->data, netdata->m_buf->len);
		}
		else if (message->m_type == MSG_SYS_NET_CONN)
		{
			shared_ptr<CNetConnMessage> netconn = dynamic_pointer_cast<CNetConnMessage>(message);
			DEBUG_LOG("Client Conn ip : %s, port : %d, opt : %d", netconn->m_conn.ip.c_str(),
					  netconn->m_conn.port,
					  netconn->m_opt);
			if (netconn->m_opt == CNetConnMessage::CONNECT_OK)
			{
				fout = fopen("client_up", "wb");
			}
		}
	}
	server.UnInit();

	return 0;
}

