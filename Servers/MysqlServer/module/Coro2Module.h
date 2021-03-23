#ifndef CORO_2_MODULE_H
#define CORO_2_MODULE_H
#include "BaseModule.h"

class MsgModule;

class Coro2Module :public BaseModule
{
public:
	Coro2Module(BaseLayer* l);
	~Coro2Module();

	// 通过 BaseModule 继承
	virtual void Init() override;

protected:

	void CoroTest1(NetMsg* sock);
	void CoroTest2(SHARE<BaseMsg>& msg, c_pull& pull, SHARE<BaseCoro>& coro);
	void CoroTest3(SHARE<BaseMsg>& msg);

private:
	MsgModule * m_msgModule;
};

#endif
