#ifndef FACTOR_MANAGER_H
#define FACTOR_MANAGER_H
#include "LoopFactor.h"
#include <unordered_map>

class FactorManager;

typedef struct LoopObject
{
	virtual void init(FactorManager* fm) = 0;
	virtual void recycle(FactorManager* fm) = 0;
}LoopObject;

class FactorManager
{
public:
	friend class Single;

	template<typename T>
	std::enable_if_t<!std::is_base_of_v<LoopObject,T>,T*> getLoopObj() {
		auto lf = GetFactor<T>();
		return lf->get();
	}

	template<typename T>
	std::enable_if_t<std::is_base_of_v<LoopObject, T>, T*> getLoopObj() {
		auto lf = GetFactor<T>();
		auto t = lf->get();
		t->init(this);
		return t;
	}

	template<typename T>
	std::enable_if_t<!std::is_base_of_v<LoopObject,T>> recycle(T* t)
	{
		auto lf = GetFactor<T>();
		lf->recycle(t);
	}

	template<typename T>
	std::enable_if_t<std::is_base_of_v<LoopObject, T>> recycle(T* t)
	{
		auto lf = GetFactor<T>();
		t->recycle(this);
		lf->recycle(t);
	}

	~FactorManager() {
		for (auto& it : m_factors)
			delete it.second;
	};
private:
	template<typename T>
	LoopFactor<T>* GetFactor()
	{
		auto it = m_factors.find(typeid(T).hash_code());
		if (it == m_factors.end())
		{
			auto lf = Single::NewLocal<LoopFactor<T>>();
			m_factors[typeid(T).hash_code()] = lf;
			return lf;
		}
		return static_cast<LoopFactor<T>*>(it->second);
	}

	FactorManager() {
		//std::cout << "new FactorManager "<< std::endl;
	};

	std::unordered_map<size_t, void*> m_factors;
};

#define GET_LOOP(T) Single::GetInstence<FactorManager>()->getLoopObj<T>();
#define RCL_LOOP(t) Single::GetInstence<FactorManager>()->recycle(t);

#endif