#ifndef UDP_NET_LAYER_H
#define UDP_NET_LAYER_H

#include "BaseLayer.h"
#include "DataDefine.h"

class MsgModule;

class LOOP_EXPORT UdpNetLayer:public BaseLayer
{
public:
	UdpNetLayer(const int32_t& port):BaseLayer(LY_UDP_NET),m_port(port)
	{};
	virtual ~UdpNetLayer()
	{};

protected:
	// Í¨¹ý BaseLayer ¼Ì³Ð
	virtual void init() override;
	virtual void afterInit() override;
	virtual void loop() override;
	virtual void close() override;
	virtual void GetDefaultTrans(int32_t & ltype, int32_t & lid) override;
private:
	int32_t m_port;
	MsgModule* m_msg;
};

#endif
