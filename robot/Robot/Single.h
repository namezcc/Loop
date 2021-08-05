#ifndef SINGLE_H
#define SINGLE_H

class Single
{
public:
	template<typename T>
	inline static T* GetInstance()
	{
		static T t;
		return &t;
	}

	template<typename T>
	inline static T* LocalInstance()
	{
		thread_local static T t;
		return &t;
	}
};

#endif