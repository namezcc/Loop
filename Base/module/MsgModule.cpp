﻿#include "MsgModule.h"
#include "ScheduleModule.h"

MsgModule::MsgModule(BaseLayer* l):BaseModule(l), m_coroIndex(0), m_coroCheckTime(0), m_curList(&m_coroList1), m_msgCash(NULL)
{
}


MsgModule::~MsgModule()
{
}

void MsgModule::Init()
{
	//m_schedule = GET_MODULE(ScheduleModule);

	GetLayer()->RegLayerMsg([this](void*msg) {
		this->MsgCallBack(msg);
	});

	AddMsgCall(L_REQUEST_CORO_MSG, BIND_SHARE_CALL(DoRequestMsg));
	AddMsgCall(L_RESPONSE_CORO_MSG, BIND_SHARE_CALL(DoResponseMsg));

	AddMsgCall(N_REQUEST_CORO_MSG, BIND_SHARE_CALL(DoNetRequestMsg));
	AddMsgCall(N_RESPONSE_CORO_MSG, BIND_SHARE_CALL(DoNetResponseMsg));

	//m_schedule->AddTimePointTask(this,&MsgModule::CheckCoroClear,-1);
}
void MsgModule::Execute()
{
	auto dt = GetSecend();
	if (dt > m_coroCheckTime)
	{
		CheckCoroClear(dt);
		m_coroCheckTime = dt + 30;	//per second check
	}
}

void MsgModule::SendMsg(const int32_t & msgid, BaseData * data)
{
	auto msg = GetLayer()->GetLayerMsg<BaseMsg>();
	msg->msgId = msgid;
	msg->m_data = data;
	GetLayer()->writePipe(msg);
}

void MsgModule::SendMsg(const int32_t& ltype, const int32_t& lid, const int32_t& msgid, BaseData* data)
{
	auto msg = GET_LAYER_MSG(BaseMsg);
	msg->msgId = msgid;
	msg->m_data = data;
	GetLayer()->writePipe(ltype,lid, msg);
}

void MsgModule::MsgCallBack(void* msg)
{
	auto smsg = (BaseMsg*)msg;

	if (smsg->msgId > L_BEGAN && smsg->msgId < N_END)
	{
		if (m_arrayCall[smsg->msgId])
			m_arrayCall[smsg->msgId](smsg);
		else
			LOOP_RECYCLE(smsg);
	}
	else if(smsg->msgId > CM_MSG_BEGIN && smsg->msgId < CM_MSG_END)
	{
		if (m_protoCall[smsg->msgId - CM_MSG_BEGIN])
			m_protoCall[smsg->msgId - CM_MSG_BEGIN](smsg);
		else
			LOOP_RECYCLE(smsg);
	}
	else
	{
		LP_ERROR << "error misId:" << smsg->msgId;
		LOOP_RECYCLE(smsg);
	}
}

void MsgModule::TransMsgCall(SHARE<NetServerMsg>& msg)
{
	auto base = GET_LOOP(BaseMsg);
	base->m_data = msg.get();
	auto smsg = SHARE<BaseMsg>(base,[this,msg](BaseMsg* nmsg){
		nmsg->m_data = NULL;
		LOOP_RECYCLE(nmsg);
	});
	if (msg->mid > L_BEGAN && msg->mid < N_END)
	{
		if (m_arrayCall[msg->mid])
		{
			m_msgCash = smsg;
			m_arrayCall[msg->mid](NULL);
		}
	}
	else if(msg->mid > CM_MSG_BEGIN && msg->mid<CM_MSG_END)
	{
		if (m_protoCall[msg->mid - CM_MSG_BEGIN])
		{
			m_msgCash = smsg;
			m_protoCall[msg->mid - CM_MSG_BEGIN](NULL);
		}
	}
}

SHARE<BaseMsg> MsgModule::RequestAsynMsg(const int32_t& mid,BaseData* data,c_pull& pull,SHARE<BaseCoro>& coro,const int32_t& ltype,const int32_t& lid)
{
	auto nid = GenCoroIndex();
	
	if (lid<0)
		RequestCoroMsg(mid,data,nid);
	else
		RequestCoroMsg(mid,data,nid,ltype,lid);
	return PullWait(nid,coro,pull);
}

SHARE<BaseMsg> MsgModule::ResponseAsynMsg(SHARE<BaseMsg>& msg,BaseData* data,c_pull& pull,SHARE<BaseCoro>& coro,const int32_t& ltype,const int32_t& lid)
{
	auto comsg = (CoroMsg*)msg.get();
	auto coid = comsg->m_mycoid >0 ? comsg->m_mycoid : comsg->m_coroId;
	auto nid = GenCoroIndex();
	if(lid<0)
		ResponseAndWait(data,coid,nid);
	else
		ResponseAndWait(data,coid,nid,ltype,lid);
	return PullWait(nid,coro,pull);
}

SHARE<BaseMsg> MsgModule::PullWait(const int32_t& coid,SHARE<BaseCoro>& coro,c_pull& pull)
{
	coro->Refresh(coid);
	(*m_curList)[coid] = coro;
	pull();
	return pull.get();
}

void MsgModule::ResponseMsg(SHARE<BaseMsg>& msg,BaseData* data,const int32_t& ltype,const int32_t& lid)
{
	auto comsg = (CoroMsg*)msg.get();
	auto coid = comsg->m_mycoid >0 ? comsg->m_mycoid : comsg->m_coroId;
	if(lid<0)
		ResponseCoroMsg(data,coid);
	else
		ResponseCoroMsg(data,coid,ltype,lid);
}

void MsgModule::CheckCoroClear(const int64_t& dt)
{
	for (auto it=m_coroList1.begin();it!=m_coroList1.end();)
	{
		if (dt >= it->second->m_endPoint)
		{
			it->second->m_endPoint = -1; //mean fail
			it->second->Clear();
			m_coroList1.erase(it++);
		}
		else
			break;
	}
	for (auto it = m_coroList2.begin(); it != m_coroList2.end();)
	{
		if (dt >= it->second->m_endPoint)
		{
			it->second->m_endPoint = -1;	//mean fail
			it->second->Clear();
			m_coroList2.erase(it++);
		}
		else
			break;
	}
}

void MsgModule::RequestCoroMsg(const int32_t& mid, BaseData* data,const int32_t& coid)
{
	auto msg = GET_LAYER_MSG(CoroMsg);
	msg->msgId = L_REQUEST_CORO_MSG;
	msg->m_subMsgId = mid;
	msg->m_data = data;
	msg->m_coroId = coid;
	GetLayer()->writePipe(msg);
}

void MsgModule::ResponseCoroMsg(BaseData* data,const int32_t& coid)
{
	auto msg = GET_LAYER_MSG(CoroMsg);
	msg->msgId = L_RESPONSE_CORO_MSG;
	msg->m_data = data;
	msg->m_coroId = coid;
	GetLayer()->writePipe(msg);
}

void MsgModule::ResponseAndWait(BaseData* data, const int32_t& coid,const int32_t& mycoid)
{
	auto msg = GET_LAYER_MSG(CoroMsg);
	msg->msgId = L_RESPONSE_CORO_MSG;
	msg->m_data = data;
	msg->m_mycoid = mycoid;
	msg->m_coroId = coid;
	GetLayer()->writePipe(msg);
}

void MsgModule::RequestCoroMsg(const int32_t& mid, BaseData* data,const int32_t& coid,const int32_t& ltype, const int32_t& lid)
{
	auto msg = GET_LAYER_MSG(CoroMsg);
	msg->msgId = L_REQUEST_CORO_MSG;
	msg->m_subMsgId = mid;
	msg->m_data = data;
	msg->m_coroId = coid;
	GetLayer()->writePipe(ltype,lid,msg);
}

void MsgModule::ResponseCoroMsg(BaseData* data,const int32_t& coid,const int32_t& ltype, const int32_t& lid)
{
	auto msg = GET_LAYER_MSG(CoroMsg);
	msg->msgId = L_RESPONSE_CORO_MSG;
	msg->m_data = data;
	msg->m_coroId = coid;
	GetLayer()->writePipe(ltype,lid,msg);
}

void MsgModule::ResponseAndWait(BaseData* data, const int32_t& coid,const int32_t& mycoid,const int32_t& ltype, const int32_t& lid)
{
	auto msg = GET_LAYER_MSG(CoroMsg);
	msg->msgId = L_RESPONSE_CORO_MSG;
	msg->m_data = data;
	msg->m_mycoid = mycoid;
	msg->m_coroId = coid;
	GetLayer()->writePipe(ltype,lid,msg);
}

void MsgModule::DoRequestMsg(SHARE<BaseMsg>& msg)
{
	CoroMsg* cmsg = (CoroMsg*)msg.get();
	if (cmsg->m_subMsgId > L_BEGAN && cmsg->m_subMsgId < N_END)
	{
		if (m_arrayCall[cmsg->m_subMsgId])
		{
			m_msgCash = msg;
			m_arrayCall[cmsg->m_subMsgId](NULL);
		}
	}
	else if(cmsg->m_subMsgId > CM_MSG_BEGIN && cmsg->m_subMsgId < CM_MSG_END)
	{
		if (m_protoCall[cmsg->m_subMsgId - CM_MSG_BEGIN])
		{
			m_msgCash = msg;
			m_protoCall[cmsg->m_subMsgId - CM_MSG_BEGIN](NULL);
		}
	}
}

void MsgModule::DoResponseMsg(SHARE<BaseMsg>& msg)
{
	CoroMsg* cmsg = (CoroMsg*)msg.get();
	int32_t coroid = cmsg->m_coroId;
	auto clist = m_curList;
	if (coroid > m_coroIndex)
		clist = GetDiffCurList();
	auto it = clist->find(coroid);
	if (it!=clist->end())
	{
		auto coro = it->second;
		clist->erase(it);
		auto& corofunc = *coro->m_coro;
		corofunc(msg);
	}
}

SHARE<CoroMsg> MsgModule::DecodeCoroMsg(SHARE<BaseMsg>& msg)
{
	auto netmsg = dynamic_cast<NetMsg*>(msg->m_data);
	if (netmsg == NULL)
	{
		LP_ERROR << "DecodeCoroMsg error m_data not NetMsg*";
		return NULL;
	}
	auto netbuff = netmsg->popBuffBlock();
	
	auto coromsg = GET_LOOP(CoroMsg);
	coromsg->m_subMsgId = netbuff->readInt32();
	coromsg->m_coroId = netbuff->readInt32();
	coromsg->m_mycoid = netbuff->readInt32();
	netmsg->push_front(netbuff);

	std::swap(coromsg->m_data,msg->m_data);
	auto coroShar = SHARE<CoroMsg>(coromsg,[this,msg](CoroMsg* ptr){
		//swap back m_data
		std::swap(ptr->m_data,msg->m_data);
		LOOP_RECYCLE(ptr);
	});
	return coroShar;
}

void MsgModule::DoNetRequestMsg(SHARE<BaseMsg>& msg)
{
	auto comsg = std::dynamic_pointer_cast<BaseMsg>(DecodeCoroMsg(msg));
	if(comsg)
		DoRequestMsg(comsg);
}

void MsgModule::DoNetResponseMsg(SHARE<BaseMsg>& msg)
{
	auto comsg = std::dynamic_pointer_cast<BaseMsg>(DecodeCoroMsg(msg));
	if (comsg)
		DoResponseMsg(comsg);
}

MsgModule::LogHook::~LogHook()
{
	//m->SendMsg(LY_LOG, 0, L_LOG_INFO, log);
	auto msg = m->GET_LAYER_MSG(BaseMsg);
	msg->msgId = L_LOG_INFO;
	msg->m_data = log;
	m->GetLayer()->writePipe(LY_LOG, 0, msg);
}
