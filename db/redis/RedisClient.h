#ifndef _H_REDIS_CLIENT_H
#define _H_REDIS_CLIENT_H

#include <stdlib.h>
#include <string>
#include <vector>
#include <hiredis.h>
#include <time.h>

using namespace std;

#define DEBUG_LOG(format,...) fprintf(stdout, "[%d][debug][%s:%d][%s]:"format"\n", time(NULL), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define ERROR_LOG(format,...) fprintf(stderr, "[%d][error][%s:%d][%s]:"format"\n", time(NULL), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

class CRefCount;
class CRedisResult;
class CRedisClient;

class CRefCount
{
public:
	CRefCount() :
		m_pCount(new int(1))
	{
	}

	CRefCount(const CRefCount& other) :
		m_pCount(other.m_pCount)
	{
		++(*m_pCount);
	}

	virtual ~CRefCount()
	{
		if (--(*m_pCount) == 0) 
		{
			delete m_pCount;
			m_pCount = 0;
		}
	}

	CRefCount& operator=(const CRefCount& other);

	bool Only() const
	{
		return (*m_pCount == 1);
	}

	int Count() const
	{
		return *m_pCount;
	}

private:
	int* m_pCount;		
};


class CRedisResult
{
	friend class CRedisClient;
public:
	CRedisResult(const CRedisResult& other) :
		m_count(other.m_count),
		m_reply(other.m_reply),
		m_isRootNode(other.m_isRootNode)
	{
	}

	~CRedisResult()
	{
		Free();
	}

	CRedisResult& operator=(const CRedisResult& other);

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

	string GetValueString() const;
	int GetValueNumber() const;
	vector<CRedisResult> GetValueArray() const;

	string ToString() const;

private:

	CRedisResult(redisReply* reply, bool root = true) :
		m_reply(reply),
		m_isRootNode(root)
	{
	}

	void Free()
	{
                if (m_reply && m_isRootNode && m_count.Only())
                {
			//DEBUG_LOG();
                        freeReplyObject(m_reply);
                        m_reply = NULL;
                }
	}

	string ToString(const vector<CRedisResult>& vecResult) const;
private:
	CRefCount m_count;
	redisReply* m_reply;
	bool  m_isRootNode;	
};

class CRedisClient
{
public:
	CRedisClient(const string& ip = "127.0.0.1", const int port = 6379) :
		m_context(NULL),
		m_serverIp(ip),
		m_serverPort(port)
	{
	}

	~CRedisClient()
	{
		Free();
	}
	
	bool Connect(const struct timeval tv);
	
	CRedisResult ExectueCommand(const char* format, ...);

	int ErrorCode() const
	{
		return (m_context ? m_context->err : 0);
	}

	string ErrorStr() const
	{
		return string(m_context ? m_context->errstr : "");
	}

private:
	void Free()
	{
		if (m_context)
		{
			redisFree(m_context);
			m_context = NULL;
		}
	}
private:
	redisContext* m_context;
	std::string m_serverIp;
	int m_serverPort;	
};

#endif
