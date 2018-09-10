#ifndef UDP_NET_MODULE_H
#define UDP_NET_MODULE_H

#include "BaseModule.h"

class UdpServerModule;
class MsgModule;
class ScheduleModule;

struct UdpBuff;

struct UdpCashNode:public LoopObject
{
	SHARE<BuffBlock> m_buff;
	SHARE<UdpBuff> m_udpbuff;

	// Í¨¹ý LoopObject ¼Ì³Ð
	virtual void init(FactorManager * fm) override;
	virtual void recycle(FactorManager * fm) override;
};

class LOOP_EXPORT UdpNetModule:public BaseModule
{
public:
	UdpNetModule(BaseLayer* l);
	~UdpNetModule();

	virtual void Init() override;

protected:

	void InitHandle();
	void OnCloseSocket(NetSocket* msg);
	void OnSocketSendData(NetMsg* nMsg);

	void TickSendBuff(int64_t& dt);

private:
	UdpServerModule* m_udpServer;
	MsgModule* m_msgModule;
	ScheduleModule* m_schedule;

	std::unordered_map<int32_t,std::list<SHARE<UdpCashNode>>> m_cashSendBuff;
};

#endif
