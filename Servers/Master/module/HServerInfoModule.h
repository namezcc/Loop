#ifndef H_SERVER_INFO_H
#define H_SERVER_INFO_H
#include "BaseModule.h"

class HttpLogicModule;
class MsgModule;
struct HttpMsg;

class HServerInfoModule:public BaseModule
{
public:
	HServerInfoModule(BaseLayer* l);
	~HServerInfoModule();

private:
	// 通过 BaseModule 继承
	virtual void Init() override;
	void OnReqGetMachineList(HttpMsg* msg);
	void OnGetMachineList(NetMsg* msg);
private:
	HttpLogicModule* m_httpModule;
	MsgModule* m_msgModule;
};

#endif