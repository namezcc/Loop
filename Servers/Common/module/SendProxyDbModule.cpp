#include "SendProxyDbModule.h"
#include "TransMsgModule.h"
#include "MsgModule.h"

SendProxyDbModule::SendProxyDbModule(BaseLayer * l):BaseModule(l)
{
}

SendProxyDbModule::~SendProxyDbModule()
{
}

void SendProxyDbModule::Init()
{
	m_tranModule = GET_MODULE(TransMsgModule);
	m_msgModule = GET_MODULE(MsgModule);

	m_proxy.serid = 0;
	m_proxy.type = LOOP_PROXY_DB;
}

void SendProxyDbModule::SendToProxyDb(google::protobuf::Message & msg, const int & hash, const int & mid)
{
	auto forbuff = GET_LAYER_MSG(BuffBlock);
	forbuff->makeRoom(sizeof(int32_t) * 2);
	PB::WriteInt(forbuff->m_buff, hash);
	PB::WriteInt(forbuff->m_buff + sizeof(int32_t), mid);
	auto buff = PB::PBToBuffBlock(GetLayer(),msg);
	forbuff->m_next = buff;
	m_tranModule->SendToServer(m_proxy, N_FORWARD_DB_PROXY, forbuff);
}

SHARE<BaseMsg> SendProxyDbModule::RequestToProxyDb(google::protobuf::Message & msg, const int & hash, const int & mid, c_pull & pull, SHARE<BaseCoro>& coro)
{
	auto forbuff = GET_LAYER_MSG(BuffBlock);
	forbuff->makeRoom(sizeof(int32_t) * 2);
	PB::WriteInt(forbuff->m_buff, hash);
	PB::WriteInt(forbuff->m_buff + sizeof(int32_t), N_REQUEST_CORO_MSG);
	auto buff = PB::PBToBuffBlock(GetLayer(), msg);

	auto coid = m_msgModule->GenCoroIndex();
	auto corobuf = m_tranModule->EncodeCoroMsg(buff, mid, coid);

	forbuff->m_next = corobuf;		//db prox buff ÔÚ corobuff Ö®Ç°
	m_tranModule->SendToServer(m_proxy, N_FORWARD_DB_PROXY, forbuff);
	return m_msgModule->PullWait(coid,coro,pull);
}
