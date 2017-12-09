#ifndef LOGIC_LAYER_H
#define LOGIC_LAYER_H
#include "BaseLayer.h"
#include "MsgModule.h"
#include "NetObjectModule.h"

class LogicLayer:public BaseLayer
{
public:
	LogicLayer() {};
	virtual ~LogicLayer() {};

protected:
	void init();
	void loop();
	void close();

	
private:

};

#endif