#ifndef FACTOR_MANAGER_H
#define FACTOR_MANAGER_H
#include <unordered_map>
#include <memory>
#include "Block.h"
#include "Single.h"

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
	static inline typename std::enable_if<!std::is_base_of<LoopObject,T>::value,T*>::type getLoopObj() {
		return Single::GetInstance<Block<T>>()->allocateNewOnce();
	}

	template<typename T>
	static inline typename std::enable_if<std::is_base_of<LoopObject, T>::value, T*>::type getLoopObj() {
		auto t = Single::GetInstance<Block<T>>()->allocateNewOnce();
		t->init(NULL);
		return t;
	}

	template<typename T>
	static inline typename std::enable_if<!std::is_base_of<LoopObject,T>::value>::type recycle(T* t)
	{
		Single::GetInstance<Block<T>>()->deallcate(t);
	}

	template<typename T>
	static inline typename std::enable_if<std::is_base_of<LoopObject, T>::value>::type recycle(T* t)
	{
		t->recycle(NULL);
		Single::GetInstance<Block<T>>()->deallcate(t);
	}

	template<typename T>
	static inline std::shared_ptr<T> GetSharedLoop()
	{
		std::shared_ptr<T> p(getLoopObj<T>(), [](T* ptr) {
			FactorManager::recycle(ptr);
		});
		return p;
	}

	~FactorManager() {
		// for (auto& it : m_factors)
		// 	delete it.second;
	};
private:

	FactorManager() {
		//std::cout << "new FactorManager "<< std::endl;
	};

	//std::unordered_map<size_t, void*> m_factors;
};

//#define GET_LOOP(T) Single::LocalInstance<FactorManager>()->getLoopObj<T>();
//#define GET_SHARE(T) Single::LocalInstance<FactorManager>()->GetSharedLoop<T>();
//#define RECYCLE(t) Single::LocalInstance<FactorManager>()->recycle(t);

#define GET_LOOP(T) FactorManager::getLoopObj<T>();
#define GET_SHARE(T) FactorManager::GetSharedLoop<T>();
#define RECYCLE(t) FactorManager::recycle(t);

#define SHARE std::shared_ptr

#endif