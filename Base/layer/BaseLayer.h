#ifndef BASE_LAYER_H
#define BASE_LAYER_H
#include <functional>
#include "LoopArray.h"
#include "Define.h"
#include "LoopServer.h"
#include <memory>

typedef std::function<void(void*)> LayerMsg;
typedef LoopArray<void*> PIPE;

class BaseModule;
struct ServerNode;
//class LoopServer;

typedef struct RWPipe
{
	RWPipe() :rpipe(NULL), wpipe(NULL) {};
	RWPipe(PIPE* r, PIPE* w):rpipe(r), wpipe(w) {};
	PIPE* rpipe;
	PIPE* wpipe;
}RWPipe;

class LOOP_EXPORT BaseLayer
{
public:
	BaseLayer(const int32_t& ltype):m_type(ltype), m_defPipe(NULL)
	{
	};
	virtual ~BaseLayer();

	void StartRun();
	inline int32_t GetType() { return m_type; };
	inline void SetlsIndex(const int32_t& idx){m_lsindex=idx;};
	inline int32_t GetlsIndex(){return m_lsindex;};

	void regPipe(int32_t ltype, PIPE* rp, PIPE* wp)
	{
		m_pipes[ltype].push_back(RWPipe(rp, wp));
	}

	void writePipe(BaseData* msg)
	{
		if (m_defPipe)
			m_defPipe->wpipe->write(msg);
		else
			RecycleLayerMsg(msg);
	}

	void writePipe(const int32_t& ltype,const int32_t& lid, BaseData* msg)
	{
		auto it = m_pipes.find(ltype);
		if (it == m_pipes.end())
		{
			assert(0);
		}
		if(lid >= 0 && lid < it->second.size())
			it->second[lid].wpipe->write(msg);
		else
			RecycleLayerMsg(msg);
	}

	void RegLayerMsg(const LayerMsg&f)
	{
		m_msgCall = f;
	}

	template<typename T>
	T* CreateModule()
	{
		if (m_modules.find(typeid(T).hash_code()) != m_modules.end())
		{
			std::cout << "double Moudle " << typeid(T).name() << "  " << typeid(T).raw_name() << std::endl;
			assert(0);
			abort();
		}
		auto md = SHARE<T>(new T(this));
		m_modules[typeid(T).hash_code()] = md;
		return md.get();
	}

	template<typename T>
	T* GetModule()
	{
		auto it = m_modules.find(typeid(T).hash_code());
		if (it == m_modules.end())
		{
			std::cout << "Null module " << typeid(T).name() << "  " << typeid(T).raw_name() << std::endl;
			assert(0);
			abort();
			return NULL;
		}
		else
			return dynamic_cast<T*>(it->second.get());
	}

	template<typename T>
	T* GetLayerMsg()
	{
		return m_server->popMsg<T>(m_lsindex);
	}
	void RecycleLayerMsg(BaseData* msg);

	inline void SetServer(ServerNode* ser) { m_serNode = ser; };
	inline ServerNode* GetServer() { return m_serNode; };
	inline void SetLoopServer(LoopServer* ser) { m_server = ser; };
	inline LoopServer* GetLoopServer() { return m_server; };
	inline std::unordered_map<int32_t, std::vector<RWPipe>>& GetPipes() { return m_pipes; };
protected:
	void startRead(const RWPipe& pipe)
	{
		void* msg = NULL;
		while (pipe.rpipe->pop(msg)) {
			m_msgCall(msg);
		}
	}

	virtual void init() = 0;
	virtual void afterInit(){};
	virtual void loop()=0;
	virtual void close()=0;

	virtual void GetDefaultTrans(int32_t& ltype,int32_t& lid)=0;
protected:
	RWPipe* m_defPipe;
	int32_t m_type;
	int32_t m_lsindex;	//在server的下标
	LayerMsg m_msgCall;
	std::unordered_map<int32_t,std::vector<RWPipe>> m_pipes;
	std::unordered_map<size_t, SHARE<BaseModule>> m_modules;
	ServerNode* m_serNode;
	LoopServer* m_server;
};

#endif