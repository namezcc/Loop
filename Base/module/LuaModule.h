#ifndef LUA_MODULE_H
#define LUA_MODULE_H

#include "BaseModule.h"
#include "LuaState.h"

COM_MOD_CLASS;

#define HOLD_1 std::placeholders::_1
#define HOLD_2 std::placeholders::_1,std::placeholders::_2

enum CTOL_INDEX
{
	CTOL_NONE = 0,
	CTOL_NET_MSG = 1,
	CTOL_MSG = 2,

	CTOL_EXPAND = 10,

	CTOL_MAX = 100,
};

enum LTOC_INDEX
{
	LTOC_NONE = 0,
	LTOC_SEND_MSG = 1,
	LTOC_SEND_SERVER = 2,

	LTOC_EXPAND,

	LTOC_MAX = 100,
};

class LOOP_EXPORT LuaModule:public BaseModule
{
public:
	LuaModule(BaseLayer* l);
	~LuaModule();

	// ͨ�� BaseModule �̳�
	virtual void Init() override;
	virtual void Execute() override;

	void runScript(const std::string& f);
	void setUpdateFunc(const std::string& f);
	void callGlobalFunc(const std::string& f,LuaArgs& arg);
	void setLuaCallFunc(const std::function<int32_t(int32_t, LuaState*)>& f);
	bool callLuaFunc(int32_t findex, LuaArgs& arg);
	bool callLuaMsg(LuaArgs& arg);
	void addCashSendBuff(BuffBlock* b);
	void removeSendBuff(BuffBlock* b);
	LuaState* getLuaState() { return m_curState; }
protected:
	LuaState* CreateLuaState();

	int onLuaSendMsg(LuaState* l);
	int onLuaSendServerMsg(LuaState* l);


private:
	LuaState* m_curState;
	std::vector<SHARE<LuaState>> m_stats;
	ServerPath m_send_path;
	//防止lua脚本报错没发消息导致内存泄露
	//lua发送消息的地方都要调用 remove
	std::set<BuffBlock*> m_buff_send_cash;

	COM_MOD_OBJ;
};

#endif
