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

#include "game/gamemsg.h"
#include "game/player.h"



#include <string.h>
#include <signal.h>


#include <iostream>
#include <ctype.h>
#include <iostream>

using namespace ctm;
using namespace std;

class CGameClient
{
public:
	CGameClient();
	~CGameClient();

	bool Init();

	void Run();

private:

	void HandleMsg(CGameMsg * pMsg);

	void HandleLoginMsgS2C(CLoginMsgS2C * pMsg);

	void ReadIni();
	
private:

	CNetTcpClient m_tcpClient;

	string m_openId;
	string m_userName;
	string m_headerImageUrl;

	CPlayer m_player;
	
};

