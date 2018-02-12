#include "RedisClient.h"
#include <stdio.h>
#include <sstream>

template <typename T>
inline std::string toString(T m)
{
    std::stringstream ss;
    ss << m;
    return ss.str();
}

CRefCount& CRefCount::operator=(const CRefCount& other) 
{
	if(m_pCount != other.m_pCount)
	{
		if(--(*m_pCount) == 0)
		{
			delete m_pCount;
			m_pCount = 0;
		}
		m_pCount = other.m_pCount;
		++(*m_pCount);
	}
	return *this;
}

CRedisResult& CRedisResult::operator=(const CRedisResult& other)
{
	if (m_reply != other.m_reply)
	{
		Free();
		m_reply = other.m_reply;
		m_count = other.m_count; 
	}
	
	return *this;
}


string CRedisResult::GetValueString() const
{
	if (IsNull()) return string("");

	return string(m_reply->str,  m_reply->len);
}

int CRedisResult::GetValueNumber() const
{
	if (IsNull()) return -1;

	return m_reply->integer;	
}

vector<CRedisResult> CRedisResult::GetValueArray() const
{
	vector<CRedisResult> vecResult;
	for (int i = 0; i < m_reply->elements; ++i)
	{
		vecResult.push_back(CRedisResult(m_reply->element[i], false));
	}
	return vecResult;
}

string CRedisResult::ToString() const
{
        if (IsNull()) return string("");

        switch(m_reply->type)
        {
        case REDIS_REPLY_STRING:
        case REDIS_REPLY_ERROR:
                return  string(m_reply->str,  m_reply->len);
        case REDIS_REPLY_INTEGER:
                return toString(m_reply->integer);
        case REDIS_REPLY_ARRAY:
        	return ToString(GetValueArray());
        }

        return string("");
}

string CRedisResult::ToString(const vector<CRedisResult>& vecResult) const
{
	string str = "[";

	if (vecResult.size() > 0)
	{
		str += vecResult[0].ToString();
	}

	for (int i = 1; i < vecResult.size(); ++i)
	{
		str += ",";
		str += vecResult[i].ToString();
	}
	str += "]";
	return str;
}

bool CRedisClient::Connect(const struct timeval tv)
{
	m_context = redisConnectWithTimeout(m_serverIp.c_str(), m_serverPort, tv);
	if (m_context == NULL || m_context->err)
	{
		return false;
	}
	return true;
}

CRedisResult CRedisClient::ExectueCommand(const char* format, ...)
{
	if (m_context == NULL) return CRedisResult(NULL);

	va_list vl;
	va_start(vl, format);
	redisReply* reply = (redisReply*)redisvCommand(m_context, format, vl);
	va_end(vl);
	
	return CRedisResult(reply);
}


int main()
{
	CRedisClient client;
	struct timeval val = {1, 500000};
	if (!client.Connect(val))
	{
		printf("connect failed!\n");
		return -1;
	}
	
	CRedisResult res = client.ExectueCommand("HGETALL set1");
	printf("res :  %s\n", res.ToString().c_str());
	res = client.ExectueCommand("SET name panda");
	printf("res :  %s\n", res.ToString().c_str());
	res = client.ExectueCommand("GET name");
	printf("res :  %s\n", res.ToString().c_str());

	res = client.ExectueCommand("HGETALL car1");
	printf("res :  %s\n", res.ToString().c_str());

	return 0;
}
