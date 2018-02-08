#ifndef MSG_MODULE_H
#define MSG_MODULE_H
#include <functional>
#include <unordered_map>
#include "BaseModule.h"
#include "spdlog/common.h"

typedef std::function<void(BaseData*)> MsgCall;

class LOOP_EXPORT MsgModule:public BaseModule
{
public:

	struct LogHook
	{
		LogHook(MsgModule* nm,const int& lv):m(nm)
		{
			log = new LogInfo();
			log->level = lv;
		}

		~LogHook()
		{
			//m->SendLogMsg(level, log);
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

	//void SendLogMsg(const int& level, stringstream& log)
	//{
	//	auto loginfo = new LogInfo();
	//	loginfo->level = level;
	//	loginfo->log = move(log);
	//	//std::initializer_list<int>{(loginfo->log<<std::forward<Args>(args),0)...};
	//	SendMsg(LY_LOG, 0, L_LOG_INFO, loginfo);
	//}
private:
	void Init();
	void Execute();
	void MsgCallBack(void* msg);

private:
	std::unordered_map<int, MsgCall> m_callBack;
};

//#define LP_TRACE(M,...) M->SendLogMsg(spdlog::level::trace,__VA_ARGS__)
//#define LP_DEBUG(M,...) M->SendLogMsg(spdlog::level::debug,__FILE__,__LINE__,__VA_ARGS__)
//#define LP_INFO(M,...) M->SendLogMsg(spdlog::level::info,__VA_ARGS__)
//#define LP_WARN(M,...) M->SendLogMsg(spdlog::level::warn,__VA_ARGS__)
//#define LP_ERROR(M,...) M->SendLogMsg(spdlog::level::err,__FILE__,__LINE__,__VA_ARGS__)

#define LP_TRACE(M) MsgModule::LogHook(M,spdlog::level::trace)
#define LP_DEBUG(M) MsgModule::LogHook(M,spdlog::level::debug)<<__FILE__<<__LINE__
#define LP_INFO(M) MsgModule::LogHook(M,spdlog::level::info)
#define LP_WARN(M) MsgModule::LogHook(M,spdlog::level::warn)
#define LP_ERROR(M) MsgModule::LogHook(M,spdlog::level::err)<<__FILE__<<__LINE__

#endif