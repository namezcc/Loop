#ifndef UTILS_H
#define UTILS_H
#include <functional>
#include <utility>
#include <map>

using namespace std;

template<int N>
struct ArgsBind;

#define ARGS_BIND(N,...)\
template<>				\
struct ArgsBind<N>		\
{						\
	template<typename R,typename F,typename T>	\
	static R Bind(F&&f,T&&t)					\
	{											\
		return bind(forward<F>(f), forward<T>(t),__VA_ARGS__);\
	}					\
};

ARGS_BIND(1, placeholders::_1);
ARGS_BIND(2, placeholders::_1, placeholders::_2);
ARGS_BIND(3, placeholders::_1, placeholders::_2, placeholders::_3);
ARGS_BIND(4, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4);
ARGS_BIND(5, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4, placeholders::_5);
ARGS_BIND(6, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4, placeholders::_5, placeholders::_6);

template<typename F>
struct FuncTool;

template<typename R,typename ...Args>
struct FuncTool<R(*)(Args...)>:ArgsBind<sizeof...(Args)>
{
	using FuncType = function<R(Args...)>;
};

template<typename R,typename T, typename ...Args>
struct FuncTool<R(T::*)(Args...)> :ArgsBind<sizeof...(Args)>
{
	using FuncType = function<R(Args...)>;
};

struct AnyFuncBind
{
	template<typename F,typename T,typename R=typename FuncTool<F>::FuncType>
	static R Bind(F&&f, T&&t)
	{
		return FuncTool<F>::template Bind<R>(forward<F>(f), forward<T>(t));
	}
};

#define ANY_BIND(t,f) AnyFuncBind::Bind(forward<F>(f), forward<T>(t))

typedef function<void()> Call;
class ExitCall
{
public:
	ExitCall(Call call):m_call(call)
	{
	};

	~ExitCall() {
		m_call();
	};
private:
	Call m_call;
};

#endif