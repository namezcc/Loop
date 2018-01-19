#include "TestLayer.h"
#include "Test/module/TestModule.h"

TestLayer::TestLayer()
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