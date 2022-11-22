#include "TcpClientModule.h"
#include "LuaState.h"
//#include "define.h"
extern "C" {
#include "mpb.h"
}

void TcpClientModule::init()
{
	m_luaState->BindLuaOrgCall("BindTcpCall", this, &TcpClientModule::BindTcpCall);
	m_luaState->BindLuaOrgCall("SendStreamData", this, &TcpClientModule::SendStreamData);
	
	m_luaState->BindLuaCall("AddTcpConnect", this, &TcpClientModule::AddConnect);
	m_luaState->BindLuaCall("SendTcpData", this, &TcpClientModule::SendData);
	m_luaState->BindLuaCall("closeConn", this, &TcpClientModule::closeConn);
}

void TcpClientModule::Loop_Once()
{
	if(m_context.stopped())
		m_context.restart();
	m_context.poll();
	
}

int TcpClientModule::BindTcpCall(lua_State * L)
{
	if(lua_gettop(L)-2 != 2)
		return m_luaState->PushArgs(false);

	if(!lua_isstring(L,-2))
		return m_luaState->PushArgs(false);

	if (!lua_isfunction(L, -1))
		return m_luaState->PushArgs(false);
	
	auto fname = lua_tostring(L, -2);
	int32_t ref = luaL_ref(L, LUA_REGISTRYINDEX);
	m_registFunc[fname] = ref;
	OnBindTcpCall(fname, ref);
	return m_luaState->PushArgs(true);
}

int TcpClientModule::AddConnect(const int32_t& id, const std::string & ip, const int32_t & port)
{
	auto conn = SHARE<TcpConn>(new TcpConn(m_context));
	bool res = conn->Connect(ip, port);
	if (res)
	{
		m_conns[id] = conn;
		conn->BindOnClose([this,id]() {
			OnClose(id);
		});

		conn->BindOnReadPack([this,id](const int32_t& mid, const int32_t& size, char* buf) {
			OnRead(id, mid, size, buf);
		});
	}
	return m_luaState->PushArgs(res);
}

int TcpClientModule::SendData(const int32_t & id, const int32_t & mid, const std::string & data)
{
	auto it = m_conns.find(id);
	if (it == m_conns.end())
		return 0;

	it->second->SendPackData(mid,data.c_str(), data.size());
	return 0;
}

int TcpClientModule::closeConn(const int32_t & id)
{
	auto it = m_conns.find(id);
	if (it == m_conns.end())
		return 0;

	it->second->Close();
	return 0;
}

int TcpClientModule::SendStreamData(lua_State * L)
{
	if (lua_gettop(L) - 2 != 3)
		return m_luaState->PushArgs(false);

	if (!lua_isinteger(L, -3) || !lua_isinteger(L, -2))
		return m_luaState->PushArgs(false);

	int32_t id = (int32_t)lua_tointeger(L, -3);
	int32_t mid = (int32_t)lua_tointeger(L, -2);
	auto it = m_conns.find(id);
	if(it == m_conns.end())
		return m_luaState->PushArgs(false);

	size_t strsize;
	auto msg = lua_tolstring(L, -1,&strsize);

	it->second->SendPackData(mid, msg, strsize);
	return m_luaState->PushArgs(true);
}

void TcpClientModule::OnRead(const int32_t & id, const int32_t & mid, const int32_t & size, char * buff)
{
	auto it = m_conns.find(id);
	if (it == m_conns.end())
		return;
	if (m_readRegidx != 0)
	{
		std::string data;
		if(buff)
			data.append(buff, size);
		m_luaState->CallRegistFunc(m_readRegidx, id, mid, data);
	}
}

void TcpClientModule::OnClose(const int32_t & id)
{
	if(m_closeRegidx != 0)
		m_luaState->CallRegistFunc(m_closeRegidx, id);
	m_conns.erase(id);
}

void TcpClientModule::OnBindTcpCall(const std::string & fname, const int32_t & regidx)
{
	if (fname == "OnRead")
		m_readRegidx = regidx;
	else if (fname == "OnClose")
		m_closeRegidx = regidx;
}
