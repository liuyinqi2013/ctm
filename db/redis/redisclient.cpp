#include "redisclient.h"
#include <stdio.h>
#include <sstream>
#include <string.h>
#include <unistd.h>
#include <ctm/common/log.h>

#ifndef DEBUG
#define DEBUG(format, ...) fprintf(stdout, format "\n", ##__VA_ARGS__)
#endif

#ifndef ERROR
#define ERROR(format, ...) fprintf(stderr, format "\n", ##__VA_ARGS__)
#endif

template <typename T>
inline std::string toString(T m)
{
	std::stringstream ss;
	ss << m;
	return ss.str();
}

CRedisResult& CRedisResult::operator=(const CRedisResult& other)
{
	if (m_count != other.m_count)
	{
		if (--*m_count == 0)
		{
			delete m_count;
			Free();
		}
		m_reply = other.m_reply;
		m_count = other.m_count;
		m_isRootNode = other.m_isRootNode;
		++*m_count;
	}
	return *this;
}

std::string CRedisResult::GetValueString() const
{
	if (IsNull()) return std::string("");

	return std::string(m_reply->str, m_reply->len);
}

int CRedisResult::GetValueNumber() const
{
	if (IsNull()) return -1;

	return m_reply->integer;
}

std::vector<CRedisResult> CRedisResult::GetValueArray() const
{
	std::vector<CRedisResult> vecResult;
	for (unsigned int i = 0; i < m_reply->elements; ++i)
	{
		vecResult.push_back(CRedisResult(m_reply->element[i], false));
	}

	return vecResult;
}

std::string CRedisResult::ToString() const
{
	if (IsNull()) return std::string("");

	switch (m_reply->type)
	{
	case REDIS_REPLY_STRING:
	case REDIS_REPLY_ERROR:
		return std::string(m_reply->str, m_reply->len);
	case REDIS_REPLY_INTEGER:
		return toString(m_reply->integer);
	case REDIS_REPLY_ARRAY:
		return ToString(GetValueArray());
	}

	return std::string("");
}

std::string CRedisResult::ToString(const std::vector<CRedisResult>& vecResult) const
{
	std::string str = "[";

	if (vecResult.size() > 0)
	{
		str += vecResult[0].ToString();
	}

	for (unsigned int i = 1; i < vecResult.size(); ++i)
	{
		str += ",";
		str += vecResult[i].ToString();
	}
	str += "]";
	return str;
}

#define CHECK_REPLY(reply) do{\
	if (!reply) return false;\
	if (reply->type == REDIS_REPLY_ERROR) {\
		ERROR("%s", std::string(reply->str, reply->len).c_str());\
		freeReplyObject(reply);\
		return false;\
	}\
} while(0)

#define FREE_REPLY(reply) freeReplyObject(reply)
#define STRING(reply) std::string(reply->str, reply->len)

bool CRedisClient::Connect(const struct timeval& tv)
{
	if (m_context == NULL)
	{
		m_context = redisConnectWithTimeout(m_serverIp.c_str(), m_serverPort, tv);
		if (m_context == NULL || m_context->err) return false;
	}
	else
	{
		if (REDIS_OK != redisReconnect(m_context)) 
		{
			ERROR("%d:%s", m_context->err, m_context->errstr);
			return false;
		}
	}

	if (!m_passwd.empty())
	{
		redisReply* reply = (redisReply*)redisCommand(m_context, "AUTH %s", m_passwd.c_str());
		if (reply && reply->type == REDIS_REPLY_ERROR)
		{
			ERROR("%s", std::string(reply->str, reply->len).c_str());
		}
		else if (!reply)
		{
			ERROR("%d:%s", m_context->err, m_context->errstr);
		}
	}

	return true;
}

CRedisResult CRedisClient::ExectueCommand(const char* format, ...)
{
	if (m_context == NULL) return CRedisResult(NULL);

	va_list vl;
	va_start(vl, format);
	redisReply* reply = (redisReply*)redisvCommand(m_context, format, vl);
	if (!reply) ERROR("%d:%s", m_context->err, m_context->errstr);
	va_end(vl);
	
	return CRedisResult(reply);
}

redisReply* CRedisClient::ExectueCommandEx(const char* format, ...)
{
	va_list vl, cpy;
	va_start(vl, format);
	va_copy(cpy, vl);
	redisReply* reply = (redisReply*)redisvCommand(m_context, format, vl);
	if (!reply)
	{
		ERROR("%d:%s", m_context->err, m_context->errstr);
		struct timeval val = {2, 0};
		if (Connect(val))
			reply = (redisReply*)redisvCommand(m_context, format, cpy);
		else
			ERROR("%d:%s", m_context->err, m_context->errstr);
	}
	va_end(vl);
	va_end(cpy);

	Show(reply);
	
	return reply;
}

void CRedisClient::Free()
{
	if (m_context)
	{
		redisFree(m_context);
		m_context = NULL;
	}
}

void CRedisClient::Show(redisReply* reply)
{
	if (reply == NULL)
	{
		DEBUG("reply is NULL");
		return;
	}

	switch (reply->type)
	{
	case REDIS_REPLY_STRING:
		DEBUG("REDIS_REPLY_STRING:%s", STRING(reply).c_str());
		break;
	case REDIS_REPLY_INTEGER:
		DEBUG("REDIS_REPLY_INTEGER:%d", reply->integer);
		break;
	case REDIS_REPLY_NIL:
		DEBUG("REDIS_REPLY_NIL");
		break;
	case REDIS_REPLY_ERROR:
		DEBUG("REDIS_REPLY_ERROR:%s", STRING(reply).c_str());
		break;
	case REDIS_REPLY_DOUBLE:
		DEBUG("REDIS_REPLY_DOUBLE:%f", reply->dval);
		break;
	case REDIS_REPLY_BOOL:
		DEBUG("REDIS_REPLY_BOOL:%d", reply->integer);
		break;
	case REDIS_REPLY_ARRAY:
		DEBUG("REDIS_REPLY_ARRAY:");
		{
			for (unsigned int i = 0; i < reply->elements; ++i)
			{
				Show(reply->element[i]);
			}
		}
		break;
	case REDIS_REPLY_VERB:
		DEBUG("REDIS_REPLY_VERB:%s", reply->vtype);
		break;
	default:
		DEBUG("OTHER TYPE:%d", reply->type);
		break;
	}
}

void CRedisClient::ToStrVec(redisReply* reply, CStrVec& out)
{
	if (!reply || reply->type != REDIS_REPLY_ARRAY) 
		return;
	out.clear();
	for (unsigned int i = 0; i < reply->elements; ++i)
	{
		out.push_back(STRING(reply->element[i]));
	}
}

void CRedisClient::ToStrKvMap(redisReply* reply, CStrKvMap& out)
{
	if (!reply || reply->type != REDIS_REPLY_ARRAY) 
		return;
	out.clear();
	redisReply* keyReply = NULL;
	for (unsigned int i = 0; i < reply->elements; ++i)
	{
		if ((i + 1) % 2 == 1)
			keyReply = reply->element[i];
		else
			out[STRING(keyReply)] = STRING(reply->element[i]);
	}
}

bool CRedisClient::Echo(const std::string& send, std::string& recv)
{
	redisReply* reply = ExectueCommandEx("ECHO %s", send.c_str());
	CHECK_REPLY(reply);
	recv.assign(reply->str, reply->len);
	FREE_REPLY(reply);
	return true;
}

bool CRedisClient::Ping()
{
	redisReply* reply = ExectueCommandEx("PING");
	CHECK_REPLY(reply);
	FREE_REPLY(reply);
	return true;
}

bool CRedisClient::Auth(const char* passwd)
{
	redisReply* reply = ExectueCommandEx("AUTH %s", passwd);
	CHECK_REPLY(reply);
	FREE_REPLY(reply);
	return true;
}

bool CRedisClient::Exists(const std::string& key)
{
	redisReply* reply = ExectueCommandEx("EXISTS %s", key.c_str());
	CHECK_REPLY(reply);
	bool val = reply->integer;
	FREE_REPLY(reply);
	return val;
}

bool CRedisClient::Type(const std::string &key,  std::string &val)
{
	redisReply* reply = ExectueCommandEx("TYPE %s", key.c_str());
	CHECK_REPLY(reply);
	val = STRING(reply);
	FREE_REPLY(reply);
	return true;
}

bool CRedisClient::BgSave()
{
	redisReply* reply = ExectueCommandEx("BGSAVE");
	CHECK_REPLY(reply);
	FREE_REPLY(reply);
	return true;
}

bool CRedisClient::Keys(const std::string& pattern, CStrVec& out)
{
	redisReply* reply = ExectueCommandEx("KEYS %s", pattern.c_str());
	CHECK_REPLY(reply);
	ToStrVec(reply, out);
	FREE_REPLY(reply);
	return true;
}

bool CRedisClient::Get(const std::string& key, std::string& val)
{
	redisReply* reply = ExectueCommandEx("GET %s", key.c_str());
	CHECK_REPLY(reply);
	val.assign(reply->str, reply->len);
	FREE_REPLY(reply);
	return true;
}

bool CRedisClient::GetSet(const std::string &key, std::string &val)
{
	redisReply* reply = ExectueCommandEx("GETSET %s %s", key.c_str(), val.c_str());
	CHECK_REPLY(reply);
	val.assign(reply->str, reply->len);
	FREE_REPLY(reply);
	return true;
}

bool CRedisClient::Set(const std::string& key, const std::string& val, uint32_t expireMillis)
{
	redisReply* reply = NULL;
	if (expireMillis == 0) 
		reply = ExectueCommandEx("SET %s %s", key.c_str(), val.c_str());
	else
		reply = ExectueCommandEx("SET %s %s PX %u", key.c_str(), val.c_str(), expireMillis);
	CHECK_REPLY(reply);
	FREE_REPLY(reply);
	return true;
}

bool CRedisClient::Del(const std::string& key)
{
	redisReply* reply = ExectueCommandEx("DEL %s", key.c_str());
	CHECK_REPLY(reply);
	FREE_REPLY(reply);
	return true;
}

bool CRedisClient::HGet(const std::string& key, const std::string& feild, std::string& val)
{
	redisReply* reply = ExectueCommandEx("HGET %s %s", key.c_str(), feild.c_str());
	CHECK_REPLY(reply);
	val.assign(reply->str, reply->len);
	FREE_REPLY(reply);
	return true;
}

bool CRedisClient::HSet(const std::string& key, const std::string& feild, const std::string& val)
{
	redisReply* reply = ExectueCommandEx("HSET %s %s %s", key.c_str(), feild.c_str(), val.c_str());
	CHECK_REPLY(reply);
	FREE_REPLY(reply);
	return true;
}

bool CRedisClient::HGetAll(const std::string& key, CStrKvMap& out)
{
	redisReply* reply = ExectueCommandEx("HGETALL %s", key.c_str());
	CHECK_REPLY(reply);
	ToStrKvMap(reply, out);
	FREE_REPLY(reply);
	return true;
}

bool CRedisClient::HKeys(const std::string& key, CStrVec& out)
{
	redisReply* reply = ExectueCommandEx("HKEYS %s", key.c_str());
	CHECK_REPLY(reply);
	ToStrVec(reply, out);
	FREE_REPLY(reply);
	return true;
}

bool CRedisClient::Incr(const std::string& key, long long& val)
{
	redisReply* reply = ExectueCommandEx("INCR %s", key.c_str());
	CHECK_REPLY(reply);
	val = reply->integer;
	FREE_REPLY(reply);
	return true;
}

bool CRedisClient::Incrby(const std::string& key, int inc, long long& val)
{
	redisReply* reply = ExectueCommandEx("INCRBY %s %d", key.c_str(), inc);
	CHECK_REPLY(reply);
	val = reply->integer;
	FREE_REPLY(reply);
	return true;
}

bool CRedisClient::Expire(const std::string &key, uint32_t seconds)
{
	redisReply* reply = ExectueCommandEx("EXPIRE %s %u", key.c_str(), seconds);
	CHECK_REPLY(reply);
	FREE_REPLY(reply);
	return true;
}

bool CRedisClient::TTL(const std::string &key, int& ttl)
{
	redisReply* reply = ExectueCommandEx("TTL %s", key.c_str());
	CHECK_REPLY(reply);
	ttl = reply->integer;
	FREE_REPLY(reply);
	return true;
}

bool CRedisClient::Select(unsigned short db)
{
	redisReply* reply = ExectueCommandEx("SELECT %u", db);
	CHECK_REPLY(reply);
	FREE_REPLY(reply);
	return true;
}

void CRedisClient::Test()
{
	CRedisClient client;
	struct timeval val = { 1, 500000 };
	if (!client.Connect(val))
	{
		DEBUG("connect failed!\n");
		return ;
	}

	CRedisResult res = client.ExectueCommand("HGETALL set1");
	DEBUG("HGETALL res :  %s", res.ToString().c_str());
	res = client.ExectueCommand("SET name panda");
	DEBUG("SET res :  %s", res.ToString().c_str());
	res = client.ExectueCommand("GET name");
	DEBUG("GET res :  %s", res.ToString().c_str());

	res = client.ExectueCommand("HGETALL car1");
	DEBUG("HGETALL res :  %s", res.ToString().c_str());
	res = client.ExectueCommand("AUTH 123456");
	DEBUG("AUTH res :  %s", res.ToString().c_str());

	std::string a;
	client.Set("BOB", "5000", 10000);
	client.Get("BOB", a);
	DEBUG("res:%s", a.c_str());
	int b = 0;
	client.TTL("BOB", b);
	DEBUG("TTL:%d", b);
	client.HSet("qvp", "name", "panda");
	client.HSet("qvp", "age", "100");
	client.HGet("qvp", "name", a);
	DEBUG("res:%s", a.c_str());
	DEBUG("Auth:%d", client.Auth("123456"));
	DEBUG("Exists:%d", client.Exists("qvp"));
	CStrKvMap out;
	client.HGetAll("qvp", out);
	CStrKvMap::iterator it = out.begin();
	for(; it != out.end(); it++)
	{
		DEBUG("%s:%s", it->first.c_str(), it->second.c_str());
	}
	client.Get("BOB", a);
	CStrVec out1;
	client.HKeys("qvp", out1);
	CStrVec::iterator it1 = out1.begin();
	for(; it1 != out1.end(); it1++)
	{
		DEBUG("%s", it1->c_str());
	}
	long long val1 = 0;
	client.Incr("ee", val1);
	DEBUG("ee:%d", val1);
}
