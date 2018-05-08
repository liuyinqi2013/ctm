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
	CLog::GetInstance()->SetLogName("test");
	CLog::GetInstance()->SetLogPath("/opt/test/ctm/log");
	
	TcpServer server("0.0.0.0", 8888);
	
	server.Run();
	
	return 0;
}

