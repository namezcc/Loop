#ifndef MSG_MODULE_H
#define MSG_MODULE_H
#include <functional>
#include <unordered_map>
#include "BaseModule.h"

typedef std::function<void(void*)> MsgCall;
typedef std::function<int()> ChooseLayer;

class MsgModule:public BaseModule
{
public:
	MsgModule(BaseLayer* l);
	~MsgModule();

	void SendMsg(const int& msgid, BaseData* data);

	template<typename Arg,typename T, typename F>
	void AddMsgCallBack(const int mId, T&&t, F&&f)
	{
		auto call = std::move(bind(forward<F>(f), forward<T>(t), std::placeholders::_1));
		m_callBack[mId] = move([call](void* msg) {
			auto mdata = static_cast<Arg*>(msg);
			call(mdata);
			//delete mdata; 上级已经delete了
		});
	}

	void TransMsgCall(NetMsg* msg);

	void SetGetLayerFunc(const ChooseLayer& func) { m_getLayerId = func; };
	void SetGetLayerFunc(ChooseLayer&& func) { m_getLayerId = std::move(func); };
private:
	void Init();
	void Execute();
	void MsgCallBack(void* msg);

private:
	std::unordered_map<int, MsgCall> m_callBack;
	ChooseLayer m_getLayerId;
};

#endif