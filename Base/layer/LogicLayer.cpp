#include "LogicLayer.h"
#include "MsgModule.h"
#include "NetObjectModule.h"
#include "ScheduleModule.h"
#include "EventModule.h"
#include <iostream>

void LogicLayer::init() {
	auto msgmd = CreateModule<MsgModule>();

	CreateModule<NetObjectModule>();
	CreateModule<ScheduleModule>();
	CreateModule<EventModule>();
};
void LogicLayer::loop() {
	//std::cout << "LogicLayer loop..." << endl;

};
void LogicLayer::close() {

}
void LogicLayer::GetDefaultTrans(int & ltype, int & lid)
{
	ltype = LY_NET;
	lid = 0;
}