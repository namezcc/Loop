#ifndef SINGLE_H
#define SINGLE_H

class Single
{
public:
	template<typename T>
	static T* GetInstence() {
		static T t;
		return &t;
	}

	template<typename T>
	static T* LocalInstance()
	{
		static thread_local T t;
		return &t;
	}
};


#endif