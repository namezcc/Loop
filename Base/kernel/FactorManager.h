#ifndef FACTOR_MANAGER_H
#define FACTOR_MANAGER_H
#include "LoopFactor.h"
#include <unordered_map>
#include <memory>
#include "Block2.h"

class FactorManager;

typedef struct LoopObject
{
	virtual void init(FactorManager*) = 0;
	virtual void recycle(FactorManager*) = 0;
}LoopObject;

class FactorManager
{
public:
	friend class Single;

	//template<typename T>
	//static typename std::enable_if<!std::is_base_of<LoopObject,T>::value,T*>::type getLoopObj() {
	//	//auto lf = GetFactor<T>();
	//	return Single::LocalInstance<LoopFactor<T>>()->get();
	//}

	//template<typename T>
	//static typename std::enable_if<std::is_base_of<LoopObject, T>::value, T*>::type getLoopObj() {
	//	//auto lf = GetFactor<T>();
	//	auto t = Single::LocalInstance<LoopFactor<T>>()->get();
	//	t->init(NULL);
	//	return t;
	//}

	//template<typename T>
	//static typename std::enable_if<!std::is_base_of<LoopObject,T>::value>::type recycle(T* t)
	//{
	//	//auto lf = GetFactor<T>();
	//	Single::LocalInstance<LoopFactor<T>>()->recycle(t);
	//}

	//template<typename T>
	//static typename std::enable_if<std::is_base_of<LoopObject, T>::value>::type recycle(T* t)
	//{
	//	//auto lf = GetFactor<T>();
	//	t->recycle(NULL);
	//	Single::LocalInstance<LoopFactor<T>>()->recycle(t);
	//}

	//template<typename T>
	//static std::shared_ptr<T> GetSharedLoop()
	//{
	//	std::shared_ptr<T> p(FactorManager::getLoopObj<T>(), [](T* ptr) {
	//		FactorManager::recycle(ptr);
	//	});
	//	return p;
	//}
	//-----------------------
	template<typename T>
	static typename std::enable_if<!std::is_base_of<LoopObject, T>::value, T*>::type getLoopObj_2() {
		return Single::LocalInstance<Block2<T>>()->allocateNewOnce();
	}

	template<typename T>
	static typename std::enable_if<std::is_base_of<LoopObject, T>::value, T*>::type getLoopObj_2() {
		auto t = Single::LocalInstance<Block2<T>>()->allocateNewOnce();
		t->init(NULL);
		return t;
	}

	template<typename T>
	static typename std::enable_if<!std::is_base_of<LoopObject, T>::value>::type recycle_2(T* t)
	{
		Single::LocalInstance<Block2<T>>()->deallcate(t);
	}

	template<typename T>
	static typename std::enable_if<std::is_base_of<LoopObject, T>::value>::type recycle_2(T* t)
	{
		t->recycle(NULL);
		Single::LocalInstance<Block2<T>>()->deallcate(t);
	}

	template<typename T>
	static std::shared_ptr<T> GetSharedLoop_2()
	{
		std::shared_ptr<T> p(FactorManager::getLoopObj_2<T>(), [](T* ptr) {
			FactorManager::recycle_2(ptr);
		});
		return p;
	}
	//-----------------------------
};

//#define GET_SHARE(T) FactorManager::GetSharedLoop<T>()
//#define GET_LOOP(T) FactorManager::getLoopObj<T>()
//#define LOOP_RECYCLE(t) FactorManager::recycle(t)

#define GET_SHARE(T) FactorManager::GetSharedLoop_2<T>()
#define GET_LOOP(T) FactorManager::getLoopObj_2<T>()
#define LOOP_RECYCLE(t) FactorManager::recycle_2(t)

#endif