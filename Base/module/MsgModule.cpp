#include "MsgModule.h"

MsgModule::MsgModule(BaseLayer* l):BaseModule(l)
{
}


MsgModule::~MsgModule()
{
}

void MsgModule::Init()
{
	GetLayer()->RegLayerMsg(&MsgModule::MsgCallBack, this);
}
void MsgModule::Execute()
{

}

void MsgModule::SendMsg(const int & msgid, BaseData * data)
{
	auto msg = new BaseMsg();
	msg->msgId = msgid;
	msg->data = data;
	GetLayer()->writePipe(msg);
}

void MsgModule::SendMsg(const int& ltype, const int& lid, const int& msgid, BaseData* data)
{
	auto msg = new BaseMsg();
	msg->msgId = msgid;
	msg->data = data;
	GetLayer()->writePipe(ltype,lid, msg);
}

void MsgModule::MsgCallBack(void* msg)
{
	auto nmsg = (BaseMsg*)msg;
	auto it = m_callBack.find(nmsg->msgId);
	if (it != m_callBack.end())
		it->second(nmsg->data);
	delete nmsg;
}

void MsgModule::TransMsgCall(NetMsg* msg)
{
	auto it = m_callBack.find(msg->mid);
	if (it != m_callBack.end())
		it->second(msg);
	delete msg;
}