#ifndef MSG_MODULE_H
#define MSG_MODULE_H
#include <functional>
#include <unordered_map>
#include "BaseModule.h"
#include "spdlog/common.h"

class ScheduleModule;

typedef std::function<void(SHARE<BaseMsg>&)> MsgCall;

class LOOP_EXPORT MsgModule:public BaseModule
{
public:

	struct LogHook
	{
		LogHook(MsgModule* nm,const int32_t& lv):m(nm)
		{
			log = nm->GetLayer()->GetLayerMsg<LogInfo>();
			log->level = lv;
		}

		~LogHook()
		{
			m->SendMsg(LY_LOG, 0, L_LOG_INFO, log);
		}

		template<typename T>
		LogHook& operator <<(T&&t)
		{
			log->log << std::forward<T>(t);
			return *this;
		}
	private:
		MsgModule* m;
		LogInfo* log;
	};

	MsgModule(BaseLayer* l);
	~MsgModule();

	void SendMsg(const int32_t& msgid, BaseData* data);
	void SendMsg(const int32_t& ltype, const int32_t& lid, const int32_t& msgid, BaseData* data);

	//typename std::enable_if<!std::is_same<F,void(T::*)(SHARE<BaseMsg>&)>::value,void>::type 
	template<typename T, typename F>
	void AddMsgCallBack(const int32_t mId, T&&t, F&&f)
	{
		/*auto call = std::move(ANY_BIND(t,f));
		m_callBack[mId] = move([call](SHARE<BaseMsg>& msg) {
			auto mdata = dynamic_cast<FuncArgsType<F>::arg1>(msg->m_data);
			if(mdata)
				call(mdata);
			else
				LogHook(this,spdlog::level::err)<<__FILE__<<__LINE__<<"Recv Msg cast Null";
		});*/
		AddMsgCallBackEx<typename FuncArgsType<F>::arg1>(mId, std::forward<T>(t), std::forward<F>(f));
	}

	/*template<typename T, typename F>
	typename std::enable_if<std::is_same<F, void(T::*)(SHARE<BaseMsg>&)>::value,void>::type
	AddMsgCallBack(const int32_t mId, T&&t, F&&f)
	{
		auto call = std::move(ANY_BIND(t,f));
		m_callBack[mId] = move([call](SHARE<BaseMsg>& msg) {
			call(msg);
		});
	}*/

	template<typename T, typename F>
	void AddAsynMsgCallBack(const int32_t& mid,T&&t,F&&f)
	{
		auto call = ANY_BIND(t,f);
		m_callBack[mid] = [this,call](SHARE<BaseMsg>& inmsg){
			auto baseco = GET_SHARE(BaseCoro);
			auto coro = new c_push([call,&baseco](c_pull& pull){
				auto msg = pull.get();	//不能用引用 防止析构
				auto pcoro = baseco;	//copy 防止跳出后析构
				call(msg,pull,pcoro);
			});
			baseco->SetCoro(coro);
			(*coro)(inmsg);
		};
	}

	void DoCoroFunc(const std::function<void(c_pull&,SHARE<BaseCoro>& coro )>& func)
	{
		auto baseco = GET_SHARE(BaseCoro);
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
protected:

	template<typename C,typename T, typename F>
	typename std::enable_if<!std::is_same<C,SHARE<BaseMsg>&>::value, void>::type
	AddMsgCallBackEx(const int32_t mId, T&&t, F&&f)
	{
		auto call = std::move(ANY_BIND(t, f));
		m_callBack[mId] = move([this,call](SHARE<BaseMsg>& msg) {
			auto mdata = dynamic_cast<C>(msg->m_data);
			if (mdata)
				call(mdata);
			else
				LogHook(this, spdlog::level::err) << __FILE__ << __LINE__ << "Recv Msg cast Null";
		});
	}

	template<typename C,typename T, typename F>
	typename std::enable_if<std::is_same<C, SHARE<BaseMsg>&>::value, void>::type
	AddMsgCallBackEx(const int32_t mId, T&&t, F&&f)
	{
		auto call = std::move(ANY_BIND(t, f));
		m_callBack[mId] = move([call](SHARE<BaseMsg>& msg) {
			call(msg);
		});
	}

private:
	typedef std::map<int32_t,SHARE<BaseCoro>> coroMap;

	void Init();
	void Execute();
	void MsgCallBack(void* msg);

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
	ScheduleModule* m_schedule;

	std::unordered_map<int32_t, MsgCall> m_callBack;

	int64_t m_coroCheckTime;
	int32_t m_coroIndex;
	coroMap m_coroList1;
	coroMap m_coroList2;
	coroMap* m_curList;
};

#define LP_TRACE(M) MsgModule::LogHook(M,spdlog::level::trace)
#define LP_DEBUG(M) MsgModule::LogHook(M,spdlog::level::debug)<<__FILE__<<__LINE__<<"\t"
#define LP_INFO(M) MsgModule::LogHook(M,spdlog::level::info)
#define LP_WARN(M) MsgModule::LogHook(M,spdlog::level::warn)
#define LP_ERROR(M) MsgModule::LogHook(M,spdlog::level::err)<<__FILE__<<__LINE__<<"\t"

#define PARSEPB_NAME_IF_FALSE(name,T,msg,msgModule)\
	T name; \
	if(!name.ParseFromArray(msg->getCombinBuff(msgModule->GetLayer())->m_buff, msg->getLen()))

#define PARSEPB_IF_FALSE(T,msg,msgModule)\
	T pbMsg; \
	if(!pbMsg.ParseFromArray(msg->getCombinBuff(msgModule->GetLayer())->m_buff, msg->getLen()))

#define TRY_PARSEPB(T,msg,msgModule) \
	T pbMsg; \
	if(!pbMsg.ParseFromArray(msg->getCombinBuff(msgModule->GetLayer())->m_buff, msg->getLen())){	\
		LP_ERROR(msgModule)<<"parse "<< #T << "error";	\
	return;}

#define TRY_PARSEPB_NAME(name,T,msg,msgModule) \
	T name; \
	if(!name.ParseFromArray(msg->getCombinBuff(msgModule->GetLayer())->m_buff, msg->getLen())){	\
		LP_ERROR(msgModule)<<"parse "<< #T << "error";	\
	return;}

#endif