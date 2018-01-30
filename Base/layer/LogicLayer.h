#ifndef LOGIC_LAYER_H
#define LOGIC_LAYER_H
#include "BaseLayer.h"
#include "MsgModule.h"
#include "NetObjectModule.h"

class LOOP_EXPORT LogicLayer:public BaseLayer
{
public:
	LogicLayer() {};
	virtual ~LogicLayer() {};

protected:
	virtual void init();
	virtual void loop();
	virtual void close();

	
private:

};

#endif