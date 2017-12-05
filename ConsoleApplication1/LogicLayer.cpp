#include "LogicLayer.h"
#include "MsgModule.h"
#include "NetObjectModule.h"
#include "ScheduleModule.h"
#include "TestModule.h"
#include <iostream>

void LogicLayer::init() {
	auto msgmd = CreateModule<MsgModule>();
	CreateModule<NetObjectModule>();
	CreateModule<ScheduleModule>();
	CreateModule<TestModule>();

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