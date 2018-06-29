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

#include <string.h>
#include <signal.h>

#include <iostream>
#include <ctype.h>
#include <iostream>
#include <map>

using namespace ctm;
using namespace std;

class CClientPlayer
{
public:
	CClientPlayer();
	~CClientPlayer()
	{
	}

public:
	string m_openId;
	string m_userName;
	string m_headerImageUrl;
	int    m_daskPos;
	int    m_status;
};

class CGameClient
{
public:
	CGameClient();
	~CGameClient();

	bool Init();
	
	bool Init(const string & serverIp, int port)
	{
		m_serverIp = serverIp;
		m_port	   = port;
		return Init();
	}

	void Run();

private:

	void HandleMsg(CGameMsg * pMsg);

	void HandleLoginMsgS2C(CLoginMsgS2C * pMsg);

	void HandleJoinGameS2C(CJoinGameS2C * pMsg);

	void HandlePlayerArrayMsgS2C(CPlayerArrayMsg * pMsg);

	void HandleGameBeginS2C(CGameBeginS2C * pMsg);

	void HandleCallDiZhuS2C(CCallDiZhuS2C * pMsg);

	void HandleOutCardsS2C(COutCardsS2C * pMsg);

	void SendMSG(CGameMsg * pMsg);

	void ShowHandCards();

	void GetOutCards(std::vector<CCard> & outCards);

	void OutCards();
	
private:

	CNetTcpClient m_tcpClient;

	string m_serverIp;
	int    m_port;

public:
	int m_zhuangPos;
	CPlayerItem m_clientPlayer;
	map<string, CPlayerItem> m_otherPlayers;
	map<int, string> m_posOpenIdMap;
	std::vector<CCard> m_handCards;
};

