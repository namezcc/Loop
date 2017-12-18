#ifndef BASE_LAYER_H
#define BASE_LAYER_H
#include <functional>
#include "LoopArray.h"
#include "FactorManager.h"
#include <memory>

#define SHARE std::shared_ptr

typedef std::function<void(void*)> LayerMsg;
typedef LoopList<void*> PIPE;

class BaseModule;
struct ServerNode;

typedef struct RWPipe
{
	RWPipe() :rpipe(NULL), wpipe(NULL) {};
	RWPipe(PIPE* r, PIPE* w):rpipe(r), wpipe(w) {};
	PIPE* rpipe;
	PIPE* wpipe;
}RWPipe;

class BaseLayer
{
public:
	BaseLayer():m_msgCall(NULL){
		++LID;
		m_id = LID;
		m_factor = Single::NewLocal<FactorManager>();
	};
	virtual ~BaseLayer();

	void StartRun();
	int GetID() { return m_id; };

	void regPipe(int layer, PIPE* rp, PIPE* wp)
	{
		m_pipes[layer] = RWPipe(rp, wp);
	}

	template<typename T>
	void writePipe(const int& lid,T* msg)
	{
		auto it = m_pipes.find(lid);
		assert(it != m_pipes.end());
		it->second.wpipe->write((void*)msg);
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
	void Recycle(T* t)
	{
		m_factor->recycle(t);
	}

	inline void SetServer(ServerNode* server) { m_server = server; };
	inline ServerNode* GetServer() { return m_server; };
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

	std::unordered_map<int, RWPipe>& GetPipes() { return m_pipes; };
private:
	int m_id;
	static int LID;
	SHARE<LayerMsg> m_msgCall;
	std::unordered_map<int, RWPipe> m_pipes;
	std::unordered_map<size_t, SHARE<BaseModule>> m_modules;
	FactorManager* m_factor;
	ServerNode* m_server;
};

#endif