#ifndef UTIL_H
#define UTIL_H

#include <functional>

template<int N>
struct ArgsBind;

template<>				
struct ArgsBind<0>
{
	template<typename R, typename F, typename T>
	static R Bind(F&&f, T&&t)
	{
		return std::bind(std::forward<F>(f), std::forward<T>(t));
	}
	template<typename R, typename F>
	static R Bind(F&&f)		
	{			
		return std::bind(std::forward<F>(f));
	}
};

#define ARGS_BIND(N,...)\
template<>				\
struct ArgsBind<N>		\
{						\
	template<typename R,typename F,typename T>	\
	static R Bind(F&&f,T&&t)					\
	{											\
		return std::bind(std::forward<F>(f), std::forward<T>(t),__VA_ARGS__);\
	}					\
	template<typename R,typename F>	\
	static R Bind(F&&f)					\
	{											\
		return std::bind(std::forward<F>(f),__VA_ARGS__);\
	}					\
};

ARGS_BIND(1, std::placeholders::_1);
ARGS_BIND(2, std::placeholders::_1, std::placeholders::_2);
ARGS_BIND(3, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
ARGS_BIND(4, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
ARGS_BIND(5, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
ARGS_BIND(6, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6);

template<typename F>
struct FuncTool;

template<typename R, typename ...Args>
struct FuncTool<R(*)(Args...)> :ArgsBind<sizeof...(Args)>
{
	using FuncType = std::function<R(Args...)>;
};

template<typename R, typename T, typename ...Args>
struct FuncTool<R(T::*)(Args...)> :ArgsBind<sizeof...(Args)>
{
	using FuncType = std::function<R(Args...)>;
};

struct AnyFuncBind
{
	template<typename F, typename T, typename R = typename FuncTool<F>::FuncType>
	static R Bind(F&&f, T&&t)
	{
		return FuncTool<F>::template Bind<R>(std::forward<F>(f), std::forward<T>(t));
	}

	template<typename F, typename R = typename FuncTool<F>::FuncType>
	static R Bind(F&&f)
	{
		return FuncTool<F>::template Bind<R>(std::forward<F>(f));
	}
};

#define ANY_BIND(t,f) AnyFuncBind::Bind(std::forward<F>(f), std::forward<T>(t))
#define ANY_BIND_2(f) AnyFuncBind::Bind(std::forward<F>(f))

template<int N, typename T>
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

template<typename R>
struct FuncArgsType<R(*)()>
{
	using typeR = R;
	static constexpr int SIZE = 0;
	using tupleArgs = void;
};

template<typename R, typename ...Args>
struct FuncArgsType<R(*)(Args...)> :ArgsTypeN<sizeof...(Args), Args...>
{
	using typeR = R;
	static constexpr int SIZE = sizeof...(Args);
	using tupleArgs = std::tuple<typename std::decay<Args>::type ...>;
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

template<int32_t N>
struct ApplyFunc
{
	template<typename R,typename F, typename T, typename ...A>
	static R apply(F&&f, T&&t, A... args)
	{
		return ApplyFunc<N - 1>::template apply<R>(std::forward<F>(f), std::forward<T>(t), std::get<N - 1>(std::forward<T>(t)), std::forward<A>(args)...);
	}
};

template<>
struct ApplyFunc<1>
{
	template<typename R,typename F, typename T, typename ...A>
	static R apply(F&&f, T&&t, A... args)
	{
		return std::forward<F>(f)(std::get<0>(std::forward<T>(t)), std::forward<A>(args)...);
	}
};

#endif
