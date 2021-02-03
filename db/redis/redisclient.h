#ifndef _H_REDIS_CLIENT_H
#define _H_REDIS_CLIENT_H

#include <stdlib.h>
#include <string>
#include <vector>
#include <stdint.h>
#include <unordered_map>
#include <hiredis/hiredis.h>

class CRedisResult;
class CRedisClient;

class CRedisResult
{
public:
	CRedisResult(const CRedisResult &other) : 
	m_count(other.m_count),
	m_reply(other.m_reply),
	m_isRootNode(other.m_isRootNode)
	{
		++*m_count;
	}

	~CRedisResult()
	{
		if (--*m_count == 0)
		{
			delete m_count;
			Free();
		}
	}

	CRedisResult &operator=(const CRedisResult &other);

	bool IsNull() const
	{
		return (NULL == m_reply);
	}

	bool IsString() const
	{
		return (m_reply && m_reply->type == REDIS_REPLY_STRING);
	}

	bool IsNumber() const
	{
		return (m_reply && m_reply->type == REDIS_REPLY_INTEGER);
	}

	bool IsArray() const
	{
		return (m_reply && m_reply->type == REDIS_REPLY_ARRAY);
	}

	std::string GetValueString() const;
	int GetValueNumber() const;
	std::vector<CRedisResult> GetValueArray() const;

	std::string ToString() const;

private:
	CRedisResult(redisReply *reply, bool root = true) : 
	m_count(new int(1)),
	m_reply(reply),
	m_isRootNode(root)
	{
	}

	void Free()
	{
		if (m_reply && m_isRootNode)
		{
			freeReplyObject(m_reply);
			m_reply = NULL;
		}
	}

	std::string ToString(const std::vector<CRedisResult> &vecResult) const;

private:
	int *m_count;
	redisReply *m_reply;
	bool m_isRootNode;

	friend class CRedisClient;
};

typedef std::vector<std::string> CStrVec;
typedef std::unordered_map<std::string, std::string> CStrKvMap;

class CRedisClient
{
public:
	CRedisClient(const std::string &ip = "127.0.0.1", const int port = 6379, const std::string &passwd = "") : 
	m_context(NULL),
	m_serverIp(ip),
	m_serverPort(port),
	m_passwd(passwd)
	{
	}

	~CRedisClient()
	{
		Free();
	}

	bool Connect(const struct timeval &tv);

	CRedisResult ExectueCommand(const char *format, ...);
	redisReply *ExectueCommandEx(const char *format, ...);

	int ErrorCode() const
	{
		return (m_context ? m_context->err : 0);
	}

	std::string ErrorStr() const
	{
		return std::string(m_context ? m_context->errstr : "");
	}

	redisContext *RedisContext()
	{
		return m_context;
	}

	static void ToStrVec(redisReply *reply, CStrVec &out);
	static void ToStrKvMap(redisReply *reply, CStrKvMap &out);
	static void Test();

public:
	bool Echo(const std::string &send, std::string &recv);
	bool Ping();
	bool Auth(const char *passwd);
	bool Exists(const std::string &key);
	bool Type(const std::string &key,  std::string &val);
	bool BgSave();
	bool Keys(const std::string &pattern, CStrVec &out);
	bool Get(const std::string &key, std::string &val);
	bool GetSet(const std::string &key, std::string &val);
	bool Set(const std::string &key, const std::string &val, uint32_t expireMillis = 0);
	bool Del(const std::string &key);
	bool HGet(const std::string &key, const std::string &feild, std::string &val);
	bool HSet(const std::string &key, const std::string &feild, const std::string &val);
	bool HGetAll(const std::string &key, CStrKvMap &out);
	bool HKeys(const std::string &key, CStrVec &out);
	bool Incr(const std::string &key, long long &val);
	bool Incrby(const std::string &key, int inc, long long &val);
	bool Expire(const std::string &key, uint32_t seconds);
	bool TTL(const std::string &key, int& ttl);
	bool Select(unsigned short db);

private:
	void Free();
	void Show(redisReply *reply);

private:
	redisContext *m_context;
	std::string m_serverIp;
	int m_serverPort;
	std::string m_passwd;
};

#endif
