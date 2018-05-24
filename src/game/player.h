#ifndef CTM_GAME_PLAYER_H__
#define CTM_GAME_PLAYER_H__

#include <string>
#include "net/soket.h"

namespace ctm
{

	class CDask;
	
	class CPlayer
	{
	public:
		CPlayer();
		virtual ~CPlayer();
		
	public:
		std::string m_openId;
		std::string m_playerName;
		std::string m_headerImageUrl;

		CSocket m_sock;
		CDask*  m_dask;
	};
}


#endif

