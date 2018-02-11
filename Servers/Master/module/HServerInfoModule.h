#ifndef H_SERVER_INFO_H
#define H_SERVER_INFO_H
#include "BaseModule.h"

class HttpLogicModule;
class MsgModule;
class HttpMsg;

class HServerInfoModule:public BaseModule
{
public:
	HServerInfoModule(BaseLayer* l);
	~HServerInfoModule();

private:
	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;
	void OnReqGetMachineList(HttpMsg* msg);
	void OnGetMachineList(NetMsg* msg);
private:
	HttpLogicModule* m_httpModule;
	MsgModule* m_msgModule;
};

#endif