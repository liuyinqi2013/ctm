#ifndef CTM_NET_NETMSG_H__
#define CTM_NET_NETMSG_H__

#include "socket.h"
#include "common/msg.h"
#include "thread/mutex.h"
#include "thread/sem.h"

#include <vector>
#include <list>
#include <map>
#include <stdlib.h>

#define BUF_MAX_SIZE 16 * 1024

namespace ctm
{
	class CNetMsg : public CMsg
	{
	public:
		CNetMsg();
		CNetMsg(int port, const std::string& ip, const CSocket& socket, const std::string& buf);
		virtual ~CNetMsg();
		
		void TestPrint();
		
	public:
		int m_iPort;
		std::string m_strIp;
		CSocket m_sock;
		std::string m_buf;
	};

	
	class CSystemNetMsg : public CMsg
	{
	public:
		CSystemNetMsg();
		CSystemNetMsg(int port, const std::string& ip, const int sock, const int opt);
		virtual ~CSystemNetMsg();
		
		void TestPrint();

		virtual const Json::Value& ToJson();

		virtual void FromJson(const Json::Value& json);
		
	public:
		int m_iPort;
		std::string m_strIp;
		int m_sock;
		int m_opt;
	};

	typedef struct CNetPack
	{
		CNetPack();
		CNetPack(const CNetPack& other);
		void TestPrint();
		void Copy(const CNetPack& other);
		
		int  sock;
		char ip[16];
		int  port;
		int  ilen;
		char ibuf[BUF_MAX_SIZE];
		int  olen;
		char obuf[BUF_MAX_SIZE];
		char temp[1024];
	} CNetPack;

	class CNetContext
	{
	public:
		CNetContext(const std::string& sep = "\r\n") : m_sep(sep) 
		{
		}
		
		~CNetContext()
		{
		}
 
		void SetSep(const std::string& sep) 
		{
			CLockOwner owner(m_mutex);
			m_sep = sep;
		}

		void Clear()
		{
			CLockOwner owner(m_mutex);
			m_mapContext.clear();
		}

		std::string Pack(const std::string& buf)
		{
			return buf + m_sep;
		}
		
		void GetCompletePack(SOCKET_T sock,  const std::string& buf, std::vector<std::string>& vecOut);
		void Remove(SOCKET_T sock);
	private:
		std::string m_sep; 
		CMutex m_mutex;
		std::map<SOCKET_T, std::string> m_mapContext;
	};

	inline std::string PackNetData(const std::string& buf, const std::string& sep = "[---@end@---]")
	{
		return buf + sep;
	}
}

#endif

