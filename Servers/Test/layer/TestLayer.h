#ifndef TEST_LAYER_H
#define TEST_LAYER_H

#include "LogicLayer.h"

class TestLayer:public LogicLayer
{
public:
	TestLayer();
	virtual ~TestLayer();
protected:
	virtual void init();
	//virtual void loop();
	virtual void close();
};

#endif