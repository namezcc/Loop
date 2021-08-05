#ifndef NOTICE_SERVER_MOCULE_H
#define NOTICE_SERVER_MOCULE_H

#include "BaseModule.h"

class MsgModule;
class TransMsgModule;
class EventModule;

class NoticeServerModule:public BaseModule
{
public:
	NoticeServerModule(BaseLayer* l);
	~NoticeServerModule();

private:
	// 通过 BaseModule 继承
	virtual void Init() override;

	void OnServerConnect(SHARE<NetServer>& ser);
	void OnServerClose(SHARE<NetServer>& ser);

	void OnReqNoticeServer(NetServerMsg* msg);

private:

	MsgModule* m_msgModule;
	TransMsgModule* m_transModule;
	EventModule* m_eventModule;

	std::unordered_map<int32_t, std::vector<ServerPath>> m_notice;
};

#endif
