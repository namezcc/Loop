#include "RedisModule.h"
#include "redisclient.h"

#define  _REDIS_CATCH_(function, line)     catch(redis::connection_error er)\
{\
    m_enable = false;\
    std::cout<< "Redis Error:"<< er.what() << " Function:" << function << " Line:" << line << std::endl;\
    return false;\
}\
catch(redis::timeout_error er)\
{\
    m_enable = false;\
    std::cout<< "Redis Error:"<< er.what() << " Function:" << function << " Line:" << line << std::endl;\
    return false;\
}\
catch(redis::protocol_error er)\
{\
    std::cout<< "Redis Error:"<< er.what() << " Function:" << function << " Line:" << line << std::endl;\
    return false;\
}\
catch(redis::key_error er)\
{\
    std::cout<< "Redis Error:"<< er.what() << " Function:" << function << " Line:" << line << std::endl;\
    return false;\
}\
catch(redis::value_error er)\
{\
    std::cout<< "Redis Error:"<< er.what() << " Function:" << function << " Line:" << line << std::endl;\
    return false;\
}\
catch (...)\
{\
return false; \
}

#define REDIS_CATCH _REDIS_CATCH_(__FUNCTION__, __LINE__)

#define REDIS_CHECK if(!m_enable) Reconnect()

#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#endif // _WIN32


RedisModule::RedisModule(BaseLayer * l):BaseModule(l)
{
	m_enable = false;
}

RedisModule::~RedisModule()
{
}

void RedisModule::Init()
{
}

void RedisModule::AfterInit()
{
}

void RedisModule::Execute()
{
}

bool RedisModule::Reconnect()
{
	return Connect(m_host,m_pass,m_port);
}

bool RedisModule::Connect(const string & host, const string& pass, const int& port)
{
	try
	{
		m_host = host;
		m_pass = pass;
		m_port = port;
		m_redis.reset(new redis::client(host, port, pass));
		m_enable = true;
	}
	catch (...)
	{
		m_enable = false;
	}
	return m_enable;
}

bool RedisModule::Del(const string & key)
{
	REDIS_CHECK;
	try
	{
		return m_redis->del(key);
	}
	REDIS_CATCH;
	return false;
}

bool RedisModule::Set(const string & key, const string & val)
{
	REDIS_CHECK;
	try
	{
		m_redis->set(key,val);
		return true;
	}
	REDIS_CATCH;
	return false;
}

bool RedisModule::Get(const string & key, string & val)
{
	REDIS_CHECK;
	try
	{
		val = move(m_redis->get(key));
		return true;
	}
	REDIS_CATCH;
	return false;
}

bool RedisModule::HSet(const string & key, const string & f, const string & v)
{
	REDIS_CHECK;
	try
	{
		return m_redis->hset(key,f,v);
	}
	REDIS_CATCH;
	return false;
}

bool RedisModule::HGet(const string & key, const string & f, string & v)
{
	REDIS_CHECK;
	try
	{
		v = move(m_redis->hget(key,f));
		return true;
	}
	REDIS_CATCH;
	return false;
}

bool RedisModule::HMSet(const string & key, const vec_str & f, const vec_str & v)
{
	REDIS_CHECK;
	try
	{
		m_redis->hmset(key,f,v);
		return true;
	}
	REDIS_CATCH;
	return false;
}

bool RedisModule::HMGet(const string & key, const vec_str & f, vec_str & v)
{
	REDIS_CHECK;
	try
	{
		m_redis->hmget(key,f,v);
		return true;
	}
	REDIS_CATCH;
	return false;
}

bool RedisModule::HDel(const string & key, const string & f)
{
	REDIS_CHECK;
	try
	{
		return m_redis->hdel(key,f);
	}
	REDIS_CATCH;
	return false;
}

bool RedisModule::HMDel(const string & key, const vec_str & f)
{
	REDIS_CHECK;
	try
	{
		return m_redis->hmdel(key,f);
	}
	REDIS_CATCH;
	return false;
}

bool RedisModule::HLength(const string & key, int & nLen)
{
	REDIS_CHECK;
	try
	{
		nLen = m_redis->hlen(key);
		return true;
	}
	REDIS_CATCH;
	return false;
}

bool RedisModule::Keys(const string & patten, vec_str & key)
{
	REDIS_CHECK;
	try
	{
		m_redis->keys(patten,key);
		return true;
	}
	REDIS_CATCH;
	return false;
}

bool RedisModule::HKeys(const string & key, vec_str & f)
{
	REDIS_CHECK;
	try
	{
		m_redis->hkeys(key,f);
		return true;
	}
	REDIS_CATCH;
	return false;
}

bool RedisModule::HVals(const string & key, vec_str & v)
{
	REDIS_CHECK;
	try
	{
		m_redis->hvals(key,v);
		return true;
	}
	REDIS_CATCH;
	return false;
}

bool RedisModule::HGetAll(const string & key, vector<pair_str>& res)
{
	REDIS_CHECK;
	try
	{
		m_redis->hgetall(key,res);
		return true;
	}
	REDIS_CATCH;
	return false;
}

bool RedisModule::ZAdd(const string & key, const string & m, const double & s)
{
	REDIS_CHECK;
	try
	{
		m_redis->zadd(key,s,m);
		return true;
	}
	REDIS_CATCH;
	return false;
}
