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
		return std::bind(std::forward<F>(f), std::forward<T>(t),__VA_ARGS__);\
	}					\
};

template<>				
struct ArgsBind<0>
{					
	template<typename R, typename F, typename T>
	static R Bind(F&&f, T&&t)
	{											
		return std::bind(std::forward<F>(f), std::forward<T>(t));
	}				
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

template<int n, typename T>
struct ArgType;
#define ARG_TYPE_N(n) template<typename T>\
struct ArgType<n,T>{ using arg##n = T; };
ARG_TYPE_N(1)
ARG_TYPE_N(2)
ARG_TYPE_N(3)
ARG_TYPE_N(4)
ARG_TYPE_N(5)
ARG_TYPE_N(6)
template<int N, typename T, typename ...Args>
struct ArgsTypeN :ArgsTypeN<N, Args...>, ArgType<N - sizeof...(Args), T>
{
};
template<int N, typename T>
struct ArgsTypeN<N, T> :ArgType<N, T>
{
};
template<typename F>
struct FuncArgsType;

template<typename R, typename ...Args>
struct FuncArgsType<R(*)(Args...)> :ArgsTypeN<sizeof...(Args), Args...>
{
	using typeR = R;
	static constexpr int SIZE = sizeof...(Args);
	using tupleArgs = std::tuple<typename std::decay<Args>::type ...>;
};

template<typename R>
struct FuncArgsType<R(*)()>
{
	using typeR = R;
	static constexpr int SIZE = 0;
	using tupleArgs = void;
};

template<typename R, typename T, typename ...Args>
struct FuncArgsType<R(T::*)(Args...)> :ArgsTypeN<sizeof...(Args), Args...>
{
	using typeR = R;
	using typeT = T;
	static constexpr int SIZE = sizeof...(Args);
	using tupleArgs = std::tuple<typename std::decay<Args>::type...>;
};

template<typename R, typename T>
struct FuncArgsType<R(T::*)()>
{
	using typeR = R;
	using typeT = T;
	static constexpr int SIZE = 0;
	using tupleArgs = void;
};

template<int32_t N>
struct ApplyFunc
{
	template<typename R, typename F, typename T, typename ...A>
	static R apply(F&&f, T&&t, A... args)
	{
		return ApplyFunc<N - 1>::template apply<R>(std::forward<F>(f), std::forward<T>(t), std::get<N - 1>(std::forward<T>(t)), std::forward<A>(args)...);
	}
};

template<>
struct ApplyFunc<1>
{
	template<typename R, typename F, typename T, typename ...A>
	static R apply(F&&f, T&&t, A... args)
	{
		return std::forward<F>(f)(std::get<0>(std::forward<T>(t)), std::forward<A>(args)...);
	}
};


template<typename T>
struct PointType
{
	using type = T;
};

template<typename T>
struct PointType<T*>
{
	using type = T;
};

#endif