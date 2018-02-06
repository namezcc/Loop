#include "TestLayer.h"
#include "TestModule.h"

TestLayer::TestLayer():LogicLayer(LY_LOGIC)
{
}


TestLayer::~TestLayer()
{
}

void TestLayer::init()
{
	CreateModule<TestModule>();

	LogicLayer::init();
}

void TestLayer::close()
{

}