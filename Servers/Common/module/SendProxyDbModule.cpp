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

void SendProxyDbModule::SendToProxyDb(google::protobuf::Message & msg, const int32_t & hash, const int32_t & mid)
{
	SendToProxyDb(msg, hash, mid, N_FORWARD_DB_PROXY);
}

void SendProxyDbModule::SendToProxyDbGroup(google::protobuf::Message & msg, const int32_t & groupId, const int32_t & mid)
{
	SendToProxyDb(msg, groupId, mid, N_FORWARD_DB_PROXY_GROUP);
}

SHARE<BaseMsg> SendProxyDbModule::RequestToProxyDb(google::protobuf::Message & msg, const int32_t & hash, const int32_t & mid, c_pull & pull, SHARE<BaseCoro>& coro)
{
	return RequestToProxyDb(msg, hash, mid, pull, coro, N_FORWARD_DB_PROXY);
}

SHARE<BaseMsg> SendProxyDbModule::RequestToProxyDbGroup(google::protobuf::Message & msg, const int32_t & groupId, const int32_t & mid, c_pull & pull, SHARE<BaseCoro>& coro)
{
	return RequestToProxyDb(msg, groupId, mid, pull, coro, N_FORWARD_DB_PROXY_GROUP);
}

void SendProxyDbModule::SendToProxyDb(google::protobuf::Message & msg, const int32_t & hash, const int32_t & mid, const int32_t & api)
{
	auto forbuff = GET_LAYER_MSG(BuffBlock);
	forbuff->makeRoom(sizeof(int32_t) * 2 + msg.ByteSize());
	forbuff->writeInt32(hash);
	forbuff->writeInt32(mid);
	forbuff->write(msg);
	m_tranModule->SendToServer(m_proxy, api, forbuff);
}

SHARE<BaseMsg> SendProxyDbModule::RequestToProxyDb(google::protobuf::Message & msg, const int32_t & hash, const int32_t & mid, c_pull & pull, SHARE<BaseCoro>& coro, const int32_t & api)
{
	auto coid = m_msgModule->GenCoroIndex();
	auto corobuf = m_tranModule->EncodeCoroMsg(NULL, mid, coid);

	auto forbuff = GET_LAYER_MSG(BuffBlock);
	forbuff->makeRoom(sizeof(int32_t) * 2 + msg.ByteSize() + corobuf->getSize());
	forbuff->writeInt32(hash);
	forbuff->writeInt32(N_REQUEST_CORO_MSG);

	forbuff->writeBuff(corobuf->m_buff, corobuf->getSize());

	forbuff->write(msg);

	m_tranModule->SendToServer(m_proxy, api, forbuff);
	return m_msgModule->PullWait(coid, coro, pull);
}