#ifndef LOGIC_LAYER_H
#define LOGIC_LAYER_H
#include "BaseLayer.h"
#include "MsgModule.h"
#include "NetObjectModule.h"

class LOOP_EXPORT LogicLayer:public BaseLayer
{
public:
	LogicLayer(const int& ltype):BaseLayer(ltype)
	{
		
	};
	virtual ~LogicLayer() {};

protected:
	virtual void init();
	virtual void loop();
	virtual void close();

	
private:


	// Í¨¹ý BaseLayer ¼Ì³Ð
	virtual void GetDefaultTrans(int & ltype, int & lid) override;

};

#endif