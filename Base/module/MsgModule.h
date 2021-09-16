#ifndef MSG_MODULE_H
#define MSG_MODULE_H
#include <functional>
#include <unordered_map>
#include "BaseModule.h"
#include "spdlog/common.h"

//class ScheduleModule;
#define CM_MSG_BEGIN 10000
#define CM_MSG_END 15000
#define MAX_CM_MSG_ID 5000

typedef std::function<void(SHARE<BaseMsg>&)> MsgCall;
typedef std::function<void(SHARE<BaseMsg>&, c_pull&, SHARE<BaseCoro>&)> AsynMsgCall;

struct LOOP_EXPORT LogHook
{
	LogHook(const int32_t& lv);
	~LogHook();

	template<typename T>
	LogHook& operator <<(T&&t)
	{
		log->log << std::forward<T>(t);
		return *this;
	}
private:
	LogInfo* log;
};

class LOOP_EXPORT MsgModule:public BaseModule
{
public:

	MsgModule(BaseLayer* l);
	~MsgModule();

	void SendMsg(const int32_t& msgid, BaseData* data);
	void SendMsg(const int32_t& ltype, const int32_t& lid, const int32_t& msgid, BaseData* data);

	template<typename T, typename F>
	void AddMsgCallBack(const int32_t mId, T&&t, F&&f)
	{
		AddMsgCallBackEx<typename FuncArgsType<F>::arg1>(mId, std::forward<T>(t), std::forward<F>(f));
	}

#define BIND_CALL(F,T) [this](SHARE<BaseMsg>& msg) { this->F(dynamic_cast<T*>(msg->m_data));}
#define BIND_NETMSG(F) [this](SHARE<BaseMsg>& msg) { this->F(dynamic_cast<NetMsg*>(msg->m_data));}
#define BIND_SERVER_MSG(F) [this](SHARE<BaseMsg>& msg) { this->F(dynamic_cast<NetServerMsg*>(msg->m_data));}
#define BIND_SHARE_CALL(F) [this](SHARE<BaseMsg>& msg) { this->F(msg);}

	void AddMsgCall(const int32_t& mId, const MsgCall& call)
	{
		PushMsgCall(mId,call);
	}

	/*template<typename T, typename F>
	void AddMsgCallBack2(const int32_t mId, T&&t, F&&f)
	{
		auto call = std::move(ANY_BIND(t, f));
		m_callBack2[mId] = move([this, call](BaseMsg* msg) {
			auto mdata = dynamic_cast<typename FuncArgsType<F>::arg1>(msg->m_data);
			if (mdata)
				call(mdata);
			else
				LogHook(this, spdlog::level::err) << __FILE__ << __LINE__ << "Recv Msg cast Null";
		});
	}*/

#define BIND_ASYN_CALL(F) [this](SHARE<BaseMsg>&m, c_pull&p, SHARE<BaseCoro>&c){F(m,p,c);}
#define BIND_ASYN_NETMSG(F) [this](SHARE<BaseMsg>&m, c_pull&p, SHARE<BaseCoro>&c){F(dynamic_cast<NetMsg*>(m->m_data),p,c);}

	void AddAsynMsgCall(const int32_t& mid,const AsynMsgCall& call)
	{
		PushMsgCall(mid,[this, call](SHARE<BaseMsg>& inmsg) {

			auto baseco = NEW_SHARE(BaseCoro);
			auto coro = new c_push([call, &baseco](c_pull& pull) {
				auto msg = pull.get();	//不能用引用 防止析构
				auto pcoro = baseco;	//copy 防止跳出后析构
				call(msg, pull, pcoro);
			});
			baseco->SetCoro(coro);
			(*coro)(inmsg);
		});
	}

	template<typename T, typename F>
	void AddAsynMsgCallBack(const int32_t& mid,T&&t,F&&f)
	{
		auto call = ANY_BIND(t,f);
		PushMsgCall(mid, [this,call](SHARE<BaseMsg>& inmsg){

			auto baseco = NEW_SHARE(BaseCoro);
			auto coro = new c_push([call,&baseco](c_pull& pull){
				auto msg = pull.get();	//不能用引用 防止析构
				auto pcoro = baseco;	//copy 防止跳出后析构
				call(msg,pull,pcoro);
			});
			baseco->SetCoro(coro);
			(*coro)(inmsg);
		});
	}

	void DoCoroFunc(const std::function<void(c_pull&,SHARE<BaseCoro>& coro )>& func)
	{
		auto baseco = NEW_SHARE(BaseCoro);
		auto coro = new c_push([func, &baseco](c_pull& pull) {
			auto msg = pull.get();	//不能用引用 防止析构
			auto pcoro = baseco;	//copy 防止跳出后析构
			func(pull, pcoro);
		});
		baseco->SetCoro(coro);
		SHARE<BaseMsg> nil;
		(*coro)(nil);
	}

	void TransMsgCall(SHARE<NetServerMsg>& msg);

	SHARE<BaseMsg> RequestAsynMsg(const int32_t& mid,BaseData* data,c_pull& pull,SHARE<BaseCoro>& coro,const int32_t& ltype=-1,const int32_t& lid=-1);
	SHARE<BaseMsg> ResponseAsynMsg(SHARE<BaseMsg>& msg,BaseData* data,c_pull& pull,SHARE<BaseCoro>& coro,const int32_t& ltype=-1,const int32_t& lid=-1);
	void ResponseMsg(SHARE<BaseMsg>& msg,BaseData* data,const int32_t& ltype=-1,const int32_t& lid=-1);
	SHARE<BaseMsg> PullWait(const int32_t& coid,SHARE<BaseCoro>& coro,c_pull& pull);

	int32_t GenCoroIndex()
	{
		++m_coroIndex;
		if (m_coroIndex > 100000000)
			m_coroIndex = 1;
		return m_coroIndex;
	}

	void MsgCallBack(void* msg);
	void setCommonCall(const MsgCall& call) {
		m_common_call = call;
	}
	//void MsgCallBack2(void* msg);
protected:

	template<typename C,typename T, typename F>
	typename std::enable_if<!std::is_same<C,SHARE<BaseMsg>&>::value, void>::type
	AddMsgCallBackEx(const int32_t mId, T&&t, F&&f)
	{
		auto call = std::move(ANY_BIND(t, f));
		PushMsgCall(mId, [this,call](SHARE<BaseMsg>& msg) {
			auto mdata = dynamic_cast<C>(msg->m_data);
			if (mdata)
				call(mdata);
			else
				LogHook(spdlog::level::err) << __FILE__ << __LINE__ << "Recv Msg cast Null";
		});
	}

	template<typename C,typename T, typename F>
	typename std::enable_if<std::is_same<C, SHARE<BaseMsg>&>::value, void>::type
	AddMsgCallBackEx(const int32_t mId, T&&t, F&&f)
	{
		auto call = std::move(ANY_BIND(t, f));
		PushMsgCall(mId, [this,call](SHARE<BaseMsg>& msg) {
			call(msg);
		});
	}

private:
	typedef std::map<int32_t,SHARE<BaseCoro>> coroMap;

	void Init();
	void Execute();

	void PushMsgCall(const int32_t& mid, const MsgCall& call)
	{
		if (mid > L_BEGAN && mid < N_END)
			m_arrayCall[mid] = call;
		else if (mid >= CM_MSG_BEGIN && mid < CM_MSG_END)
		{
			m_protoCall[mid - CM_MSG_BEGIN] = call;
		}
	}

	void CheckCoroClear(const int64_t& dt);
	void RequestCoroMsg(const int32_t& mid, BaseData* data,const int32_t& coid);
	void ResponseCoroMsg(BaseData* data,const int32_t& coid);
	void ResponseAndWait(BaseData* data, const int32_t& coid,const int32_t& mycoid);
	void RequestCoroMsg(const int32_t& mid, BaseData* data,const int32_t& coid,const int32_t& ltype, const int32_t& lid);
	void ResponseCoroMsg(BaseData* data,const int32_t& coid,const int32_t& ltype, const int32_t& lid);
	void ResponseAndWait(BaseData* data, const int32_t& coid,const int32_t& mycoid,const int32_t& ltype, const int32_t& lid);

	void DoRequestMsg(SHARE<BaseMsg>& msg);
	void DoResponseMsg(SHARE<BaseMsg>& msg);

	SHARE<CoroMsg> DecodeCoroMsg(SHARE<BaseMsg>& msg);
	void DoNetRequestMsg(SHARE<BaseMsg>& msg);
	void DoNetResponseMsg(SHARE<BaseMsg>& msg);

private:

	MsgCall m_arrayCall[N_END];
	MsgCall m_protoCall[MAX_CM_MSG_ID];

	MsgCall m_common_call;

	int64_t m_coroCheckTime;
	int32_t m_coroIndex;
	coroMap m_coroList;
	Loop::mlist<BaseCoro> m_coroLink;
};

#define LP_TRACE LogHook(spdlog::level::trace)
#define LP_DEBUG LogHook(spdlog::level::debug)<<__FILE__<<__LINE__<<"\t"
#define LP_INFO LogHook(spdlog::level::info)
#define LP_WARN LogHook(spdlog::level::warn)
#define LP_ERROR LogHook(spdlog::level::err)<<__FILE__<<__LINE__<<"\t"
#define LP_ERROR_SP LogHook(spdlog::level::err)

#define PARSEPB_NAME_IF_FALSE(name,T,msg)\
	T name; \
	if(!name.ParseFromArray(msg->getNetBuff(), msg->m_buff->getUnReadSize()))

#define PARSEPB_IF_FALSE(T,msg)\
	T pbMsg; \
	if(!pbMsg.ParseFromArray(msg->getNetBuff(), msg->m_buff->getUnReadSize()))

#define TRY_PARSEPB(T,msg) \
	T pbMsg; \
	if(!pbMsg.ParseFromArray(msg->getNetBuff(), msg->m_buff->getUnReadSize())){	\
		LP_ERROR<<"parse "<< #T << "error";	\
	return;}

#define TRY_PARSEPB_NAME(name,T,msg) \
	T name; \
	if(!name.ParseFromArray(msg->getNetBuff(), msg->m_buff->getUnReadSize())){	\
		LP_ERROR<<"parse "<< #T << "error";	\
	return;}

#endif