#ifndef REDIS_MODULE_H
#define REDIS_MODULE_H
#include "BaseModule.h"

namespace redis {
	struct default_hasher;
	
	template<typename HS>
	class base_client;

	typedef base_client<default_hasher> client;
}

class MsgModule;
typedef vector<string> vec_str;
typedef pair<string, string> pair_str;

class LOOP_EXPORT RedisModule:public BaseModule
{
public:
	RedisModule(BaseLayer* l);
	~RedisModule();

	inline bool IsConnect() { return m_enable; };

	void SetConnect(const string& host, const string& pass, const int& port = 6379);
	bool Connect(const string& host, const string& pass, const int& port=6379);

	bool Del(const string& key);

	bool Set(const string& key, const string& val);
	bool Get(const string& key, string& val);

	bool HSet(const string& key, const string& f, const string& v);
	bool HGet(const string& key,const string& f, string& v);
	bool HMSet(const string& key, const vec_str& f, const vec_str& v);
	bool HMSet(const string& key,const std::map<std::string, std::string>& vals);
	bool HMGet(const string& key, const vec_str& f,vec_str& v);

	bool HDel(const string& key, const string& f);
	bool HMDel(const string& key, const vec_str& f);
	bool HLength(const string& key, int& nLen);

	bool Keys(const string& patten, vec_str& key);
	bool HKeys(const string& key, vec_str& f);
	bool HVals(const string& key, vec_str& v);

	bool HGetAll(const string& key, vector<pair_str>& res);
	bool HGetAll(const string& key, std::map<std::string, std::string>& res);

	bool ZAdd(const string& key, const string& m, const double& s);

	bool SAdd(const string& key, const string& val);
	bool SMembers(const string& key, vec_str& r);
	bool SRem(const string& key, const string& val);

	//设置多少秒后过期
	bool expire(const string& key, uint32_t sec);
	bool haveKey(const string& key);
private:
	// 通过 BaseModule 继承
	virtual void Init() override;
	virtual void AfterInit() override;
	virtual void Execute() override;

	bool Reconnect();

protected:

	string m_host;
	int m_port;
	string m_pass;

	bool m_enable;

	MsgModule* m_msgModule;

	SHARE<redis::client> m_redis;
};

#endif