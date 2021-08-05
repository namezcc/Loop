#include "LoginModule.h"
#include "NetObjectModule.h"
#include "MsgModule.h"
#include "Crypto/crchash.h"
#include "GameReflectData.h"
#include "MysqlModule.h"
#include "SendProxyDbModule.h"
#include "RedisModule.h"
#include "RoomStateModule.h"
#include "TransMsgModule.h"
#include "EventModule.h"
#include "ScheduleModule.h"

#include "protoPB/base/LPSql.pb.h"
#include "protoPB/client/login.pb.h"
#include "protoPB/client/define.pb.h"
#include "protoPB/server/server.pb.h"
#include "protoPB/server/dbdata.pb.h"

LoginModule::LoginModule(BaseLayer* l):BaseModule(l)
{
}

LoginModule::~LoginModule()
{
}

void LoginModule::Init()
{
	m_msgModule= GET_MODULE(MsgModule);
	m_netModule= GET_MODULE(NetObjectModule);
	m_sendProxyDb = GET_MODULE(SendProxyDbModule);
//	m_redisModule = GET_MODULE(RedisModule);
	m_roomModule = GET_MODULE(RoomStateModule);
	m_transModule = GET_MODULE(TransMsgModule);
	m_eventModule = GET_MODULE(EventModule);
	m_schedule = GET_MODULE(ScheduleModule);


	m_msgModule->AddAsynMsgCall(LPMsg::CM_LOGIN,BIND_ASYN_CALL(OnClientLogin));

	m_msgModule->AddMsgCall(501, BIND_NETMSG(OnTestPing));
	m_msgModule->AddAsynMsgCall(500, BIND_ASYN_NETMSG(onTestAsyncPing));

	m_eventModule->AddEventCall(E_SOCKEK_CONNECT,BIND_EVENT(OnClientConnect,int32_t));


	m_lock_server.type = SERVER_TYPE::LOOP_LOGIN_LOCK;
	m_lock_server.serid = 0;

	m_db_path.push_back(*GetLayer()->GetServer());
	m_db_path.push_back({SERVER_TYPE::LOOP_MYSQL_ACCOUNT,0});
}

void LoginModule::OnClientConnect(const int32_t& sock)
{
	m_netModule->AcceptConn(sock);
}

void LoginModule::OnTestPing(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::LoginLock, msg);
	m_transModule->SendToServer(m_lock_server, 501, pbMsg);
}

void LoginModule::onTestAsyncPing(NetMsg * msg, c_pull & pull, SHARE<BaseCoro>& coro)
{
	TRY_PARSEPB(LPMsg::LoginLock, msg);
	//auto ack = m_transModule->RequestServerAsynMsg(m_lock_server, 500, pbMsg, pull, coro);

	//TRY_PARSEPB_NAME(pback,LPMsg::LoginLock, msg);
	//m_transModule->ResponseServerMsg(m_lock_server, pull.get(), pback);
	m_transModule->ResponseServerMsg(m_lock_server, pull.get(), pbMsg);
}

void LoginModule::OnClientLogin(SHARE<BaseMsg>& comsg, c_pull & pull, SHARE<BaseCoro>& coro)
{
	auto msg = (NetMsg*)comsg->m_data;
	TRY_PARSEPB(LPMsg::ReqLogin, msg);

	if (pbMsg.account().empty())
	{
		LP_INFO << "Empty Account Name";
		return;
	}

	auto itc = m_tmpClient.find(pbMsg.account());
	if (itc != m_tmpClient.end())
	{
		LP_WARN << "allread wait to login account:" << pbMsg.account();
		if(itc->second.sock != msg->socket)
			m_netModule->CloseNetObject(msg->socket);
		return;
	}

	ClientObj client = {};
	client.sock = msg->socket;
	client.pass = pbMsg.password();
	m_tmpClient[pbMsg.account()] = client;

	ExitCall failcall([this,&coro,&pbMsg]() {
		if (!coro->IsFail())
			return;
		RemoveClient(pbMsg.account());
	});

	auto hash = common::Hash32(pbMsg.account());
	LPMsg::LoginLock lockpb;
	lockpb.set_pid(hash);

	auto acklock = m_transModule->RequestServerAsynMsg(m_lock_server, N_LOGIN_LOCK, lockpb, pull, coro);
	{
		PARSEPB_NAME_IF_FALSE(acklockpb,LPMsg::LoginLock, acklock)
		{
			coro->SetFail();
			return;
		}
		if (acklockpb.pid() == 0)
		{
			coro->SetFail();
			LP_WARN << "login locked accont " << pbMsg.account();
			return;
		}
	}

	auto ackaccount = m_transModule->RequestServerAsynMsg(m_db_path, N_GET_ACCOUNT_INFO, pbMsg, pull, coro);
	{
		PARSEPB_NAME_IF_FALSE(ackpb, LPMsg::DB_account, ackaccount)
		{
			return coro->SetFail();
		}

		LP_INFO << ackpb.ShortDebugString();

		auto room = m_roomModule->GetRandRoom();
		if (room == NULL)
		{
			LP_ERROR << "no room server";
			return coro->SetFail();
		}

		LPMsg::RoomInfo roominfo;
		roominfo.set_ip(room->ip);
		roominfo.set_port(room->port);
		roominfo.set_pid(ackpb.game_uid());

		m_transModule->SendToServer(m_roomModule->GetRoomPath(room->server_id), N_ROOM_READY_TAKE_PLAYER, roominfo);
		m_netModule->SendNetMsg(msg->socket, LPMsg::SM_LOGIN_RES, roominfo);
		RemoveClient(pbMsg.account());
	}

	m_transModule->SendToServer(m_lock_server, N_LOGIN_UNLOCK, lockpb);
}

void LoginModule::RemoveClient(const std::string & account)
{
	auto it = m_tmpClient.find(account);
	if (it == m_tmpClient.end())
		return;
	m_netModule->CloseNetObject(it->second.sock);
	m_tmpClient.erase(it);
}

