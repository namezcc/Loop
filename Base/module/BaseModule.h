#ifndef BASE_MODULE_H
#define BASE_MODULE_H
#include "BaseLayer.h"
#include "MsgDefine.h"
#include "DataDefine.h"
#include "Utils.h"
#include "LPStringUtil.h"
#include <iostream>

using namespace std;
class LOOP_EXPORT BaseModule
{
public:
	BaseModule(BaseLayer* layer):m_layer(layer) {
	};
	virtual ~BaseModule() {

	};

	inline BaseLayer* GetLayer() { return m_layer; };

	virtual void Init() = 0;
	virtual void AfterInit() 
	{};
	virtual void BeforExecute()
	{};
	virtual void Execute() = 0;
private:
	BaseLayer* m_layer;
};

#endif