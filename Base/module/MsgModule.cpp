#include "MsgModule.h"
#include "BaseLayer.h"

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

void MsgModule::SendMsg(const int& msgid, void* data)
{
	auto msg = new BaseMsg();
	msg->msgId = msgid;
	msg->data = data;
	auto layerid = m_getLayerId();
	GetLayer()->writePipe(layerid, msg);
}

void MsgModule::MsgCallBack(void* msg)
{
	auto nmsg = static_cast<BaseMsg*>(msg);
	auto it = m_callBack.find(nmsg->msgId);
	if (it != m_callBack.end())
		it->second(nmsg->data);
	else
		assert(0);//delete nmsg->data;
	delete nmsg;
}

void MsgModule::TransMsgCall(NetMsg* msg)
{
	auto it = m_callBack.find(msg->mid);
	if (it != m_callBack.end())
		it->second(msg);
	else
		delete msg;
}