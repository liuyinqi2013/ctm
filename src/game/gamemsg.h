#ifndef CTM_GAME_GAMEMSG_H__
#define CTM_GAME_GAMEMSG_H__

#include <string>
#include <vector>
#include "common/msg.h"

namespace ctm
{
	class CLoginMsg : public CMsg
	{
	public:
		CLoginMsg();
		virtual ~CLoginMsg();

		virtual Json::Value ToJson() const;

		virtual void FromJson(const Json::Value& json);
		
	public:
	};

	class CLogOutMsg : public CMsg
	{
	public:
		CLogOutMsg();
		virtual ~CLogOutMsg();

		virtual Json::Value ToJson() const;

		virtual void FromJson(const Json::Value& json);
	public:
	};
}

#endif

