#include "NoticeServerModule.h"
#include "MsgModule.h"
#include "TransMsgModule.h"
#include "EventModule.h"

#include "protoPB/server/common.pb.h"

#include "ServerMsgDefine.h"

NoticeServerModule::NoticeServerModule(BaseLayer * l):BaseModule(l)
{
}

NoticeServerModule::~NoticeServerModule()
{
}

void NoticeServerModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);
	m_transModule = GET_MODULE(TransMsgModule);
	m_eventModule = GET_MODULE(EventModule);

	m_eventModule->AddEventCallBack(E_SERVER_CONNECT, this, &NoticeServerModule::OnServerConnect);
	m_eventModule->AddEventCallBack(E_SERVER_CLOSE, this, &NoticeServerModule::OnServerClose);

	m_msgModule->AddMsgCallBack(N_REQ_NOTICE_SERVER, this, &NoticeServerModule::OnReqNoticeServer);
}

void NoticeServerModule::OnServerConnect(SHARE<NetServer>& ser)
{
	auto it = m_notice.find(ser->type);
	if (it == m_notice.end())
		return;

	LPMsg::AckNoticeServer msg;
	msg.set_state(CONN_STATE::CONNECT);
	msg.add_serid(ser->serid);
	msg.set_sertype(ser->type);
	for (auto& p:it->second)
	{
		m_transModule->SendToServer(p, N_ACK_NOTICE_SERVER, msg);
	}
}

void NoticeServerModule::OnServerClose(SHARE<NetServer>& ser)
{
	auto it = m_notice.find(ser->type);
	if (it == m_notice.end())
		return;

	LPMsg::AckNoticeServer msg;
	msg.set_state(CONN_STATE::CLOSE);
	msg.add_serid(ser->serid);
	msg.set_sertype(ser->type);
	for (auto& p : it->second)
	{
		m_transModule->SendToServer(p, N_ACK_NOTICE_SERVER, msg);
	}
}

void NoticeServerModule::OnReqNoticeServer(NetServerMsg * msg)
{
	TRY_PARSEPB(LPMsg::ReqNoticeServer, msg);
	if (pbMsg.path_size() < 2)
	{
		LP_ERROR << "ReqNoticeServer path size too short";
		return;
	}

	VecPath path;
	for (auto& n:pbMsg.path())
	{
		auto ser = GET_SHARE(ServerNode);
		ser->serid = n.serid();
		ser->type = n.sertype();
		path.push_back(ser);
	}

	auto it = m_notice.find(pbMsg.servertype());
	if (it == m_notice.end())
	{
		m_notice[pbMsg.servertype()].push_back(path);
	}
	else
	{
		for (auto& p:it->second)
		{
			if (p.size() == path.size() && p.back()->serid == path.back()->serid &&
				p.back()->type == path.back()->type)
			{
				return;
			}
		}
		it->second.push_back(path);
	}

	auto& serlist = m_transModule->GetServerList();
	auto itlist = serlist.find(pbMsg.servertype());
	if (itlist != serlist.end() && itlist->second.size()>0)
	{
		LPMsg::AckNoticeServer ackmsg;
		ackmsg.set_state(CONN_STATE::CONNECT);
		ackmsg.set_sertype(pbMsg.servertype());

		for (auto& ser:itlist->second)
		{
			ackmsg.add_serid(ser.second->serid);
		}
		m_transModule->SendBackServer(msg->path, N_ACK_NOTICE_SERVER, ackmsg);
	}
}
