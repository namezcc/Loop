#include "LogicLayer.h"
#include "MsgModule.h"
#include "NetObjectModule.h"
#include "ScheduleModule.h"
#include "EventModule.h"
#include "TransMsgModule.h"
#include <iostream>
#include "LoopServer.h"

void LogicLayer::init() {
	auto msgmd = CreateModule<MsgModule>();

	auto netmod = CreateModule<NetObjectModule>();
	CreateModule<ScheduleModule>();
	CreateModule<EventModule>();
	CreateModule<TransMsgModule>();

	if (m_type != LY_LOGIC)
		return;

	// auto config = GetLoopServer()->GetConfig();
	// for (auto& ser:config.connect)
	// 	netmod->AddServerConn(ser.type, ser.serid, ser.ip, ser.port);

};
void LogicLayer::loop() {
	//std::cout << "LogicLayer loop..." << endl;

};
void LogicLayer::close() {

}
void LogicLayer::GetDefaultTrans(int32_t & ltype, int32_t & lid)
{
	ltype = LY_NET;
	lid = 0;
}