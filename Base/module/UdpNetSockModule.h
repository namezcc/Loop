﻿#ifndef UDP_NET_SOCK_MODULE_H
#define UDP_NET_SOCK_MODULE_H

#include "BaseModule.h"

class MsgModule;
class EventModule;

class LOOP_EXPORT UdpNetSockModule:public BaseModule
{
public:
	UdpNetSockModule(BaseLayer* l);
	~UdpNetSockModule();

	void SendNetMsg(const int& socket, const int& mid, const google::protobuf::Message& pbmsg);
	void SendNetMsg(const int& socket, const int32_t & mid, BuffBlock* buff);
	void BroadNetMsg(std::vector<int32_t>& socks, const int32_t & mid, const gpb::Message& pbmsg);
	void BroadNetMsg(std::vector<int32_t>& socks, const int32_t & mid, BuffBlock* buff);
	void CloseNetObject(const int& socket);
private:
	// 通过 BaseModule 继承
	virtual void Init() override;

	void OnSocketConnet(NetMsg* sock);
	void OnSocketClose(NetMsg* sock);

private:
	MsgModule * m_msgModule;
	EventModule* m_eventModule;
};

#endif
