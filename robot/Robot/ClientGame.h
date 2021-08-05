#ifndef CLIENT_GAME_H
#define CLIENT_GAME_H

#include "TcpClientModule.h"
#include "LuaState.h"

class InputCmd;

class ClientGame
{
public:
	ClientGame():m_input(NULL), m_cmdFuncReg(LUA_REFNIL)
	{};
	~ClientGame() {};

	void init();
	void Run(const int64_t& dt);

	bool LoadMainScripet(const std::string& file, const std::string& mainfunc);
	bool RunScript(const std::string& file);
	SHARE<LuaState>& GetLuaState() { return m_luaState; };
	void SetInputCmd(InputCmd* input) { m_input = input; };

	void BindCmdLuaFunc(const std::string& cmdfunc);

protected:

	void UpdateCmd();

private:
	SHARE<TcpClientModule> m_tcpmodule;
	SHARE<LuaState> m_luaState;
	int32_t m_cmdFuncReg;
	InputCmd* m_input;
};

#endif
