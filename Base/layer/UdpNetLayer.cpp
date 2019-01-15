#include "UdpNetLayer.h"
#include "MsgModule.h"
#include "ScheduleModule.h"
#include "UdpNetModule.h"
#include "UdpServerModule.h"

void UdpNetLayer::init()
{
	m_msg = CreateModule<MsgModule>();
	CreateModule<ScheduleModule>();
	CreateModule<UdpNetModule>();
	CreateModule<UdpServerModule>()->Listen(m_port);
}

void UdpNetLayer::afterInit()
{
	//RegLayerMsg(&MsgModule::MsgCallBack2, m_msg);
}

void UdpNetLayer::loop()
{
}

void UdpNetLayer::close()
{
}

void UdpNetLayer::GetDefaultTrans(int32_t & ltype, int32_t & lid)
{
	auto it = m_pipes.begin();
	ltype = it->first;
	lid = 0;
}
