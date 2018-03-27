#ifndef SEND_PROXY_DB_MODULE_H
#define SEND_PROXY_DB_MODULE_H

#include "BaseModule.h"
#include "Reflection.h"

class TransMsgModule;

class SendProxyDbModule:public BaseModule
{
public:
	SendProxyDbModule(BaseLayer* l);
	~SendProxyDbModule();

	void SendToProxyDb(google::protobuf::Message& msg,const int& hash,const int& mid);


	template<typename T>
	SHARE<T> CreateObject(vector<string>& field, vector<string>& val)
	{
		if (field.size() != val.size())
			return nullptr;

		SHARE<T> t = GetLayer()->GetSharedLoop<T>();
		for (size_t i = 0; i < field.size(); i++)
		{
			Reflect<T>::SetFieldValue(*t.get(),field[i],val[i]);
		}
		return t;
	}

private:

	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;


private:
	TransMsgModule * m_tranModule;

	ServerNode m_proxy;
};

#endif