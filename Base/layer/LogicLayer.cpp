#include "LogicLayer.h"
#include "MsgModule.h"
#include "NetObjectModule.h"
#include "ScheduleModule.h"
#include "EventModule.h"
#include "TransMsgModule.h"
#include <iostream>

void LogicLayer::init() {
	auto msgmd = CreateModule<MsgModule>();
	CreateModule<NetObjectModule>();
	CreateModule<ScheduleModule>();
	CreateModule<EventModule>();
	CreateModule<TransMsgModule>();

	msgmd->SetGetLayerFunc([this]() {
		auto it = GetPipes().begin();
		return it->first;
	});
};
void LogicLayer::loop() {
	//std::cout << "LogicLayer loop..." << endl;

};
void LogicLayer::close() {

};