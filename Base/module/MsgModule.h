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

typedef std::function<void(BaseMsg*)> MsgCall;
typedef std::function<void(SHARE<BaseMsg>&, c_pull&, SHARE<BaseCoro>&)> AsynMsgCall;

class LOOP_EXPORT MsgModule:public BaseModule
{
public:

	struct LOOP_EXPORT LogHook
	{
		LogHook(BaseModule* nm,const int32_t& lv):m(nm)
		{
			log = nm->GetLayer()->GetLayerMsg<LogInfo>();
			log->level = lv;
		}

		~LogHook();

		template<typename T>
		LogHook& operator <<(T&&t)
		{
			log->log << std::forward<T>(t);
			return *this;
		}
	private:
		BaseModule * m;
		LogInfo* log;
	};

	MsgModule(BaseLayer* l);
	~MsgModule();

	void SendMsg(const int32_t& msgid, BaseData* data);
	void SendMsg(const int32_t& ltype, const int32_t& lid, const int32_t& msgid, BaseData* data);

	template<typename T, typename F>
	void AddMsgCallBack(const int32_t mId, T&&t, F&&f)
	{
		AddMsgCallBackEx<typename FuncArgsType<F>::arg1>(mId, std::forward<T>(t), std::forward<F>(f));
	}

#define BIND_CALL(F,T) [this](BaseMsg* msg) { this->F(dynamic_cast<T*>(msg->m_data));}
#define BIND_SHARE_CALL(F) [this](BaseMsg* msg) { this->F(SHARE<BaseMsg>(msg,[](BaseMsg*){}));}

	void AddMsgCall(const int32_t& mId, const MsgCall& call)
	{
		PushMsgCall(mId,[this,call](BaseMsg* msg) {
			if (msg)
			{
				call(msg);
				RECYCLE_LAYER_MSG(msg);
			}
			else
			{
				assert(m_msgCash);
				call(m_msgCash.get());
				m_msgCash.reset();
			}
		});
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

	void AddAsynMsgCall(const int32_t& mid,const AsynMsgCall& call)
	{
		PushMsgCall(mid,[this, call](BaseMsg* inmsg) {
			SHARE<BaseMsg> comsg = GetRealMsg(inmsg);

			auto baseco = NEW_SHARE(BaseCoro);
			auto coro = new c_push([call, &baseco](c_pull& pull) {
				auto msg = pull.get();	//不能用引用 防止析构
				auto pcoro = baseco;	//copy 防止跳出后析构
				call(msg, pull, pcoro);
			});
			baseco->SetCoro(coro);
			(*coro)(comsg);
		});
	}

	template<typename T, typename F>
	void AddAsynMsgCallBack(const int32_t& mid,T&&t,F&&f)
	{
		auto call = ANY_BIND(t,f);
		PushMsgCall(mid, [this,call](BaseMsg* inmsg){
			SHARE<BaseMsg> comsg = GetRealMsg(inmsg);

			auto baseco = NEW_SHARE(BaseCoro);
			auto coro = new c_push([call,&baseco](c_pull& pull){
				auto msg = pull.get();	//不能用引用 防止析构
				auto pcoro = baseco;	//copy 防止跳出后析构
				call(msg,pull,pcoro);
			});
			baseco->SetCoro(coro);
			(*coro)(comsg);
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
		{
			m_coroIndex = 1;
			m_curList = GetDiffCurList();
		}
		return m_coroIndex;
	}

	void MsgCallBack(void* msg);
	//void MsgCallBack2(void* msg);
protected:

	template<typename C,typename T, typename F>
	typename std::enable_if<!std::is_same<C,SHARE<BaseMsg>&>::value, void>::type
	AddMsgCallBackEx(const int32_t mId, T&&t, F&&f)
	{
		auto call = std::move(ANY_BIND(t, f));
		PushMsgCall(mId, [this,call](BaseMsg* msg) {
			if (msg)
			{
				auto mdata = dynamic_cast<C>(msg->m_data);
				if (mdata)
					call(mdata);
				else
					LogHook(this, spdlog::level::err) << __FILE__ << __LINE__ << "Recv Msg cast Null";
				RECYCLE_LAYER_MSG(msg);
			}
			else
			{
				auto smsg = GetRealMsg(msg);
				auto mdata = dynamic_cast<C>(smsg->m_data);
				if (mdata)
					call(mdata);
				else
					LogHook(this, spdlog::level::err) << __FILE__ << __LINE__ << "Recv Msg cast Null";
			}
		});
	}

	template<typename C,typename T, typename F>
	typename std::enable_if<std::is_same<C, SHARE<BaseMsg>&>::value, void>::type
	AddMsgCallBackEx(const int32_t mId, T&&t, F&&f)
	{
		auto call = std::move(ANY_BIND(t, f));
		PushMsgCall(mId, [this,call](BaseMsg* msg) {
			call(GetRealMsg(msg));
		});
	}

	SHARE<BaseMsg> GetRealMsg(BaseMsg* msg)
	{
		if (msg)
		{
			return SHARE<BaseMsg>(msg, [this](BaseMsg* nmsg) { GetLayer()->RecycleLayerMsg(nmsg);});
		}
		else
		{
			assert(m_msgCash);
			SHARE<BaseMsg> smsg = m_msgCash;
			m_msgCash.reset();
			return smsg;
		}
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

	coroMap* GetDiffCurList()
	{
		return m_curList == &m_coroList1 ? &m_coroList2 : &m_coroList1;
	}

private:

	SHARE<BaseMsg> m_msgCash;

	MsgCall m_arrayCall[N_END];
	MsgCall m_protoCall[MAX_CM_MSG_ID];

	int64_t m_coroCheckTime;
	int32_t m_coroIndex;
	coroMap m_coroList1;
	coroMap m_coroList2;
	coroMap* m_curList;
};

#define LP_TRACE MsgModule::LogHook(this,spdlog::level::trace)
#define LP_DEBUG MsgModule::LogHook(this,spdlog::level::debug)<<__FILE__<<__LINE__<<"\t"
#define LP_INFO MsgModule::LogHook(this,spdlog::level::info)
#define LP_WARN MsgModule::LogHook(this,spdlog::level::warn)
#define LP_ERROR MsgModule::LogHook(this,spdlog::level::err)<<__FILE__<<__LINE__<<"\t"

#define PARSEPB_NAME_IF_FALSE(name,T,msg)\
	T name; \
	if(!name.ParseFromArray(msg->getNetBuff(), msg->getLen()))

#define PARSEPB_IF_FALSE(T,msg)\
	T pbMsg; \
	if(!pbMsg.ParseFromArray(msg->getNetBuff(), msg->getLen()))

#define TRY_PARSEPB(T,msg) \
	T pbMsg; \
	if(!pbMsg.ParseFromArray(msg->getNetBuff(), msg->getLen())){	\
		LP_ERROR<<"parse "<< #T << "error";	\
	return;}

#define TRY_PARSEPB_NAME(name,T,msg) \
	T name; \
	if(!name.ParseFromArray(msg->getNetBuff(), msg->getLen())){	\
		LP_ERROR<<"parse "<< #T << "error";	\
	return;}

#endif