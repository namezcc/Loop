#ifndef TCP_CLIENT_MODULE_H
#define TCP_CLIENT_MODULE_H

#include "TcpConn.h"
#include <map>

class LuaState;
struct lua_State;

class TcpClientModule
{
public:
	TcpClientModule(LuaState* l):m_luaState(l),m_readRegidx(0), m_closeRegidx(0)
	{};
	~TcpClientModule() {};

	void init();
	void Loop_Once();

	//lua call
	int BindTcpCall(lua_State* L);
	int AddConnect(const int32_t& id,const std::string& ip,const int32_t& port);
	int SendData(const int32_t& id, const int32_t& mid, const std::string& data);
	int closeConn(const int32_t& id);
	int SendStreamData(lua_State* L);

protected:

	void OnRead(const int32_t& id, const int32_t& mid, const int32_t& size, char* buff);
	void OnClose(const int32_t& id);
	void OnBindTcpCall(const std::string& fname, const int32_t& regidx);

private:
	LuaState * m_luaState;
	as::io_context m_context;

	std::map<int32_t, SHARE<TcpConn>> m_conns;
	std::map<std::string, int32_t> m_registFunc;

	int32_t m_readRegidx;
	int32_t m_closeRegidx;
};


#endif
