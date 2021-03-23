#ifndef UDP_NET_MODULE_H
#define UDP_NET_MODULE_H

#include "BaseModule.h"

class UdpServerModule;
class MsgModule;
class ScheduleModule;

struct UdpBuff;

struct UdpCashNode:public LoopObject
{
	SHARE<LocalBuffBlock> m_buff;
	SHARE<UdpBuff> m_udpbuff;

	// 通过 LoopObject 继承
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
	void OnCloseSocket(NetMsg* msg);
	void OnSocketSendData(NetMsg* nMsg);
	void OnBroadData(BroadMsg* nMsg);

	void TickSendBuff(int64_t& dt);

private:
	UdpServerModule* m_udpServer;
	MsgModule* m_msgModule;
	ScheduleModule* m_schedule;

	std::unordered_map<int32_t,std::vector<SHARE<UdpCashNode>>> m_cashSendBuff;
};

#endif
