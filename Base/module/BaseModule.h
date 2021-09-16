#ifndef BASE_MODULE_H
#define BASE_MODULE_H
#include "BaseLayer.h"
#include "MsgDefine.h"
#include "BaseMsg.h"
#include "DataDefine.h"
#include "Utils.h"
#include "LPStringUtil.h"
#include "ToolFunction.h"
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
	virtual void Execute()
	{};
private:
	BaseLayer* m_layer;
};

#define GET_MODULE(M) GetLayer()->GetModule<M>()

//#define GET_SHARE(T) GetLayer()->GetSharedLoop<T>()
//#define GET_LOOP(T) GetLayer()->GetLoopObj<T>()
//#define LOOP_RECYCLE(t) GetLayer()->Recycle(t)

//#define GET_SHARE(T) FactorManager::GetSharedLoop<T>()
//#define GET_LOOP(T) FactorManager::getLoopObj<T>()
//#define LOOP_RECYCLE(t) FactorManager::recycle(t)

//#define GET_LAYER_MSG(T) GetLayer()->GetLayerMsg<T>()
#define GET_LAYER_MSG(T) MsgPool::popMsg<T>()
#define RECYCLE_LAYER_MSG(t) GetLayer()->RecycleLayerMsg(t)

#define COM_MOD_CLASS class MsgModule;class TransMsgModule;class NetObjectModule;
#define COM_MOD_OBJ MsgModule* m_msg_mod;TransMsgModule* m_trans_mod;NetObjectModule* m_net_mod;

#define COM_MOD_INIT m_msg_mod = GET_MODULE(MsgModule); \
m_trans_mod = GET_MODULE(TransMsgModule); \
m_net_mod = GET_MODULE(NetObjectModule);

#endif