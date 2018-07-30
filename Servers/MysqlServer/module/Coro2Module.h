#ifndef CORO_2_MODULE_H
#define CORO_2_MODULE_H
#include "BaseModule.h"

class MsgModule;

class Coro2Module :public BaseModule
{
public:
	Coro2Module(BaseLayer* l);
	~Coro2Module();

	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;

protected:

	void CoroTest1(NetSocket* sock);
	void CoroTest2(SHARE<BaseMsg>& msg, c_pull& pull, SHARE<BaseCoro>& coro);
	void CoroTest3(SHARE<BaseMsg>& msg);

private:
	MsgModule * m_msgModule;
};

#endif
