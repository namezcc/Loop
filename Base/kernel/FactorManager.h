#ifndef FACTOR_MANAGER_H
#define FACTOR_MANAGER_H
#include "LoopFactor.h"
#include <unordered_map>
#include <memory>

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
	typename std::enable_if<!std::is_base_of<LoopObject,T>::value,T*>::type getLoopObj() {
		auto lf = GetFactor<T>();
		return lf->get();
	}

	template<typename T>
	typename std::enable_if<std::is_base_of<LoopObject, T>::value, T*>::type getLoopObj() {
		auto lf = GetFactor<T>();
		auto t = lf->get();
		t->init(this);
		return t;
	}

	template<typename T>
	typename std::enable_if<!std::is_base_of<LoopObject,T>::value>::type recycle(T* t)
	{
		auto lf = GetFactor<T>();
		lf->recycle(t);
	}

	template<typename T>
	typename std::enable_if<std::is_base_of<LoopObject, T>::value>::type recycle(T* t)
	{
		auto lf = GetFactor<T>();
		t->recycle(this);
		lf->recycle(t);
	}

	template<typename T>
	std::shared_ptr<T> GetSharedLoop()
	{
		std::shared_ptr<T> p(getLoopObj<T>(), [this](T* ptr) {
			recycle(ptr);
		});
		return p;
	}

	~FactorManager() {
		// for (auto& it : m_factors)
		// 	delete it.second;
	};
private:
	template<typename T>
	LoopFactor<T>* GetFactor()
	{
		// auto it = m_factors.find(typeid(T).hash_code());
		// if (it == m_factors.end())
		// {
		// 	auto lf = Single::NewLocal<LoopFactor<T>>();
		// 	m_factors[typeid(T).hash_code()] = lf;
		// 	return lf;
		// }
		//return static_cast<LoopFactor<T>*>(it->second);
		return Single::LocalInstance<LoopFactor<T>>();
	}

	FactorManager() {
		//std::cout << "new FactorManager "<< std::endl;
	};

	//std::unordered_map<size_t, void*> m_factors;
};

#endif