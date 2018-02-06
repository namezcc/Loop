#ifndef MSG_MODULE_H
#define MSG_MODULE_H
#include <functional>
#include <unordered_map>
#include "BaseModule.h"

typedef std::function<void(BaseData*)> MsgCall;

class LOOP_EXPORT MsgModule:public BaseModule
{
public:
	MsgModule(BaseLayer* l);
	~MsgModule();

	void SendMsg(const int& msgid, BaseData* data);
	void SendMsg(const int& ltype, const int& lid, const int& msgid, BaseData* data);

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
private:
	void Init();
	void Execute();
	void MsgCallBack(void* msg);

private:
	std::unordered_map<int, MsgCall> m_callBack;
};

#endif