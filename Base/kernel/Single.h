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
	static T* NewLocal()
	{
		return new T();
	}
private:
	Single() {};
	~Single() {};
};


#endif