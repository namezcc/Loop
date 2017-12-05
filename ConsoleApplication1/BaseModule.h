#ifndef BASE_MODULE_H
#define BASE_MODULE_H
#include "BaseLayer.h"
#include "MsgDefine.h"
#include "DataDefine.h"

using namespace std;
class BaseModule
{
public:
	BaseModule(BaseLayer* layer):m_layer(layer) {
	};
	virtual ~BaseModule() {

	};

	virtual void Init() = 0;
	virtual void Execute() = 0;

	inline BaseLayer* GetLayer() { return m_layer; };
private:
	BaseLayer* m_layer;
};

#endif