#ifndef BASE_LAYER_H
#define BASE_LAYER_H
#include <functional>
#include "LoopArray.h"
#include "FactorManager.h"
#include "Define.h"
#include <memory>

typedef std::function<void(void*)> LayerMsg;
typedef LoopList<void*> PIPE;

class BaseModule;
struct ServerNode;
class LoopServer;

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
	BaseLayer(const int& ltype):m_msgCall(NULL),m_type(ltype)
	{
		m_factor.reset(Single::NewLocal<FactorManager>());
	};
	virtual ~BaseLayer();

	void StartRun();
	inline int GetType() { return m_type; };

	void regPipe(int ltype, PIPE* rp, PIPE* wp)
	{
		m_pipes[ltype].push_back(RWPipe(rp, wp));
	}

	void writePipe(void* msg)
	{
		int ltype, lid;
		GetDefaultTrans(ltype, lid);
		writePipe(ltype, lid, msg);
	}

	void writePipe(const int& ltype,const int& lid, void* msg)
	{
		auto it = m_pipes.find(ltype);
		assert(it != m_pipes.end());
		it->second[lid].wpipe->write(msg);
	}

	template<typename F,typename T>
	void RegLayerMsg(F&&f,T&&t)
	{
		if (!m_msgCall)
			m_msgCall = SHARE<LayerMsg>(new LayerMsg(bind(forward<F>(f), forward<T>(t), placeholders::_1)));
	}

	template<typename T>
	T* CreateModule()
	{
		if (m_modules.find(typeid(T).hash_code()) != m_modules.end())
			assert(0);
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
			assert(0);
			return NULL;
		}
		else
			return dynamic_cast<T*>(it->second.get());
	}

	template<typename T>
	T* GetLoopObj()
	{
		return m_factor->getLoopObj<T>();
	}

	template<typename T>
	std::shared_ptr<T> GetSharedLoop()
	{
		return m_factor->GetSharedLoop<T>();
	}

	template<typename T>
	void Recycle(T* t)
	{
		m_factor->recycle(t);
	}

	inline void SetServer(ServerNode* ser) { m_serNode = ser; };
	inline ServerNode* GetServer() { return m_serNode; };
	inline void SetLoopServer(LoopServer* ser) { m_server = ser; };
	inline LoopServer* GetLoopServer() { return m_server; };
	inline std::unordered_map<int, std::vector<RWPipe>>& GetPipes() { return m_pipes; };
protected:
	void startRead(const RWPipe& pipe)
	{
		if (!m_msgCall)
			return;
		readMsg(pipe);
	}

	void readMsg(const RWPipe& pipe)
	{
		while (true)
		{
			PIPE::LPList l;
			if (!pipe.rpipe->read(l))
				break;
			auto head = l.getHead();
			while (head)
			{
				auto n = head->next;
				m_msgCall.get()->operator()(head->data);
				head = n;
			}
		}
	}

	virtual void init() = 0;
	virtual void loop()=0;
	virtual void close()=0;

	virtual void GetDefaultTrans(int& ltype,int& lid)=0;
protected:
	int m_type;
	SHARE<LayerMsg> m_msgCall;
	std::unordered_map<int,std::vector<RWPipe>> m_pipes;
	std::unordered_map<size_t, SHARE<BaseModule>> m_modules;
	SHARE<FactorManager> m_factor;
	ServerNode* m_serNode;
	LoopServer* m_server;
};

#endif