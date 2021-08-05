#include "ClientGame.h"
#include "LuaState.h"
#include "InputCmd.h"

void ClientGame::init()
{
	m_luaState = SHARE<LuaState>(new LuaState());
	m_tcpmodule = SHARE<TcpClientModule>(new TcpClientModule(m_luaState.get()));

	m_tcpmodule->init();
}

void ClientGame::Run(const int64_t& dt)
{
	m_luaState->RunUpdateFunc(dt);
	m_tcpmodule->Loop_Once();
	UpdateCmd();
}

bool ClientGame::LoadMainScripet(const std::string & file, const std::string & mainfunc)
{
	if (!m_luaState)
		return false;

	m_luaState->RunScript(file);
	m_luaState->CallGlobalLuaFunc(mainfunc);
	return true;
}

bool ClientGame::RunScript(const std::string & file)
{
	if (!m_luaState)
		return false;

	m_luaState->RunScript(file);
	return true;
}

void ClientGame::BindCmdLuaFunc(const std::string & cmdfunc)
{
	m_cmdFuncReg = m_luaState->RegistGlobalFunc(cmdfunc);
}

void ClientGame::UpdateCmd()
{
	if (m_input == NULL || m_cmdFuncReg==LUA_REFNIL)
		return;

	InputPam* pam = NULL;
	while (pam = m_input->PopInputCmd())
	{
		m_luaState->CallRegistFunc(m_cmdFuncReg, pam->pams);
		m_input->PushPamCash(pam);
		pam = NULL;
	}
}

