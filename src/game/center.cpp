#include "center.h"
#include "dask.h"
#include "player.h"
#include "common/log.h"

namespace ctm
{
	CGameCenter::CGameCenter(const std::string& ip, int port) :
		m_tcpNetServer(NULL),
		m_totalDaskNum(2),
		m_daskArray(NULL),
		m_ip(ip),
		m_port(port)
		
	{
	}

	CGameCenter::~CGameCenter()
	{
	}

	bool CGameCenter::Init()
	{
		m_tcpNetServer =  new CTcpNetServer(m_ip, m_port);
		if (!m_tcpNetServer)
		{
			DEBUG_LOG("Create netserver failed!");
			return false;
		}

		if (m_tcpNetServer.Init())
		{
			DEBUG_LOG("Net server init failed!");
			return false;
		}

		m_daskArray = new CDask[m_totalDaskNum];
		if (!m_daskArray)
		{
			DEBUG_LOG("Create dask array failed!");
			return false;
		}

		for (int i = 0; i < m_totalDaskNum; ++i)
		{
			m_daskArray[i].m_daskId = i + 1;
			m_vecFreeDask.push_back(m_daskArray + i);
		}
		
		return true;
	}
	
}