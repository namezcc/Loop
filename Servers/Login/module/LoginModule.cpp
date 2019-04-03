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

#include "protoPB/base/LPSql.pb.h"
#include "protoPB/client/login.pb.h"
#include "protoPB/client/define.pb.h"
#include "protoPB/server/server.pb.h"

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
	m_redisModule = GET_MODULE(RedisModule);
	m_roomModule = GET_MODULE(RoomStateModule);
	m_transModule = GET_MODULE(TransMsgModule);
	m_eventModule = GET_MODULE(EventModule);

	m_msgModule->AddAsynMsgCall(LPMsg::N_REQ_LOGIN,BIND_ASYN_CALL(OnClientLogin));
	m_msgModule->AddAsynMsgCall(N_ML_CREATE_ACCOUNT, BIND_ASYN_CALL(OnCreateAccount));

	m_msgModule->AddMsgCall(LPMsg::N_REQ_PLAYER_OPERATION, BIND_CALL(OnTestPing, NetMsg));

	m_eventModule->AddEventCall(E_SOCKEK_CONNECT,BIND_EVENT(OnClientConnect,int32_t));
}

void LoginModule::OnClientConnect(const int32_t& sock)
{
	m_netModule->AcceptConn(sock);
}

void LoginModule::OnTestPing(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::propertyInt32, msg);
	LPMsg::propertyInt32 pong;
	pong.set_data(pbMsg.data() + 1);
	m_netModule->SendNetMsg(msg->socket, LPMsg::N_ACK_OPERATION_SIZE, pong);
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
		if(itc->second->sock != msg->socket)
			m_netModule->CloseNetObject(msg->socket);
		return;
	}

	auto client = GET_SHARE(ClientObj);
	client->sock = msg->socket;
	client->pass = pbMsg.password();
	m_tmpClient[pbMsg.account()] = client;

	ExitCall failcall([this,&coro,&pbMsg]() {
		if (!coro->IsFail())
			return;
		RemoveClient(pbMsg.account());
	});

	auto hash = common::Hash32(pbMsg.account());
	LPMsg::LoginLock lockpb;
	lockpb.set_pid(hash);
	ServerNode lockser{ SERVER_TYPE::LOOP_LOGIN_LOCK,1 };
	auto acklock = m_transModule->RequestServerAsynMsg(lockser, N_LOGIN_LOCK, lockpb, pull, coro);
	{
		auto netmsg = (NetMsg*)acklock->m_data;
		PARSEPB_NAME_IF_FALSE(acklockpb,LPMsg::LoginLock, netmsg)
		{
			coro->SetFail();
			return;
		}
		if (acklockpb.pid() == 0)
		{
			coro->SetFail();
			return;
		}
	}

	if (!m_redisModule->IsConnect())
	{
		coro->SetFail();
		LP_ERROR << "Redis Connect invalid";
		return;
	}

	string sid;
	m_redisModule->HGet("Account:" + pbMsg.account(), "id", sid);

	if (sid.empty())
	{
		CreateAccount(pbMsg.account(), pbMsg.password(),pull,coro);
		return;
	}
	// unlock
	m_transModule->SendToServer(lockser, N_LOGIN_UNLOCK, lockpb);

	int64_t id = loop_cast<int64_t>(sid);

	auto account = GET_SHARE(AccoutInfo);
	account->id = id;
	account->name = pbMsg.account();
	account->pass = pbMsg.password();
	TryPlayerLogin(client, account, pull, coro);
}

void LoginModule::OnGetAccountInfo(SHARE<BaseMsg>& comsg, c_pull & pull, SHARE<BaseCoro>& coro)
{
	auto msg = (NetMsg*)comsg->m_data;
	TRY_PARSEPB(LPMsg::PBSqlParam, msg);

	vector<string> field(pbMsg.field().begin(), pbMsg.field().end());
	vector<string> value(pbMsg.value().begin(), pbMsg.value().end());

	SHARE<AccoutInfo> account = m_sendProxyDb->CreateObject<AccoutInfo>(field,value);
	if (!account)
	{
		coro->SetFail();
		return;
	}

	auto it = m_tmpClient.find(account->name);
	if (it == m_tmpClient.end())
		return;
	auto client = it->second;
	if (client->pass == account->pass)
	{
		//LP_WARN(m_msgModule) << "Account:" << account->name << "  Login success";
		auto inuse = account->lastRoomId >> 16;
		RoomServer* room = NULL;
		if (inuse)
		{
			room = m_roomModule->GetRandRoom(account->lastRoomId & 0xFF);
			if (room)
			{
				SendRoomInfo(account, client,room, pull, coro);
				return;
			}
		}
		room = m_roomModule->GetRandRoom();
		if (room)
		{
			Reflect<AccoutInfo> rf(account.get());
			rf.Set_lastRoomId((1 << 16) | room->id);

			auto ackmsg = m_sendProxyDb->RequestUpdateDbGroup(rf, account->id, account->id >> 56, N_MYSQL_CORO_MSG, pull, coro);

			auto sqlres = (NetMsg *)ackmsg->m_data;
			TRY_PARSEPB_NAME(pbsqlmsg,LPMsg::PBSqlParam, sqlres);
			if (!pbsqlmsg.ret())
			{
				LP_ERROR << "Update Account roomid Fail";
				room = NULL;
			}
		}
		SendRoomInfo(account, client,room, pull, coro);
	}
	else
	{
		RemoveClient(account->name);
		LP_WARN<< "Account:" << account->name << "  Login pass Error";
	}
}

void LoginModule::CreateAccount(const string & name, const string & pass, c_pull & pull, SHARE<BaseCoro>& coro)
{
	int hash = common::Hash32(name);

	LPMsg::PBSqlParam param;
	param.set_uid(hash);
	param.set_table(Reflect<AccoutInfo>::Name());

	param.add_kname("name");
	param.add_kval(name);

	param.set_opt(SQL_OPT::SQL_INSERT_SELECT);
	param.set_reply(N_ML_CREATE_ACCOUNT);

	param.add_field("name");
	param.add_value(name);
	param.add_field("pass");
	param.add_value(pass);

	m_sendProxyDb->SendToProxyDb(param, hash, N_CREATE_ACCOUNT);
	//LP_WARN(m_msgModule) << "Account:" << name << "Begin Create";
}

void LoginModule::OnCreateAccount(SHARE<BaseMsg>& comsg, c_pull & pull, SHARE<BaseCoro>& coro)
{
	auto msg = (NetMsg *)comsg->m_data;
	TRY_PARSEPB(LPMsg::PBSqlParam, msg);

	if (!pbMsg.ret())
	{
		LP_ERROR << "CreateAccount Fail";
		return;
	}

	vector<string> field(pbMsg.field().begin(), pbMsg.field().end());
	vector<string> value(pbMsg.value().begin(), pbMsg.value().end());

	SHARE<AccoutInfo> account = m_sendProxyDb->CreateObject<AccoutInfo>(field, value);
	if (!account)
	{
		LP_ERROR << "OnCreateAccount get account error";
		return;
	}

	bool res = m_redisModule->HMSet("Account:" + account->name, field, value);
	// unlock
	LPMsg::LoginLock lockpb;
	lockpb.set_pid(pbMsg.uid());
	ServerNode lockser{ SERVER_TYPE::LOOP_LOGIN_LOCK,1 };
	m_transModule->SendToServer(lockser, N_LOGIN_UNLOCK, lockpb);

	ExitCall failcall([this, &coro, &account]() {
		if (!coro->IsFail())
			return;
		RemoveClient(account->name);
	});

	if (!res)
	{
		RemoveClient(account->name);
		LP_ERROR << "Redis connect fail";
		return;
	}

	//LP_WARN(m_msgModule) << "Account:" << account->name << " Create Success";

	auto it = m_tmpClient.find(account->name);
	if (it != m_tmpClient.end())
		TryPlayerLogin(it->second, account, pull, coro,true);
}

bool LoginModule::TryPlayerLogin(SHARE<ClientObj>& client, SHARE<AccoutInfo>& account, c_pull & pull, SHARE<BaseCoro>& coro,bool isCreate)
{
	LPMsg::LoginLock msg;
	msg.set_pid(account->id);
	ServerNode ser{ SERVER_TYPE::LOOP_LOGIN_LOCK,1 };

	//ÇëÇóËø
	auto ackmsg = m_transModule->RequestServerAsynMsg(ser, N_LOGIN_LOCK, msg, pull, coro);
	auto netmsg = (NetMsg*)ackmsg->m_data;
	{
		PARSEPB_IF_FALSE(LPMsg::LoginLock, netmsg)
		{
			RemoveClient(account->name);
			return false;
		}
		if (pbMsg.pid() == 0)
		{
			RemoveClient(account->name);
			return false;
		}
	}

	if (!isCreate)
	{
		LPMsg::PBSqlParam param;
		param.set_uid(account->id);
		param.set_opt(SQL_OPT::SQL_SELECT);
		param.set_table(Reflect<AccoutInfo>::Name());

		param.add_kname("id");
		param.add_kval(std::to_string(account->id));

		auto comsg = m_sendProxyDb->RequestToProxyDbGroup(param, account->id >> 56, N_MYSQL_CORO_MSG, pull, coro);
		OnGetAccountInfo(comsg, pull, coro);
	}
	else{
		auto room = m_roomModule->GetRandRoom();
		if (room)
		{
			Reflect<AccoutInfo> rf(account.get());
			rf.Set_lastRoomId((1 << 16) | room->id);

			auto ackmsg = m_sendProxyDb->RequestUpdateDbGroup(rf, account->id, account->id >> 56, N_MYSQL_CORO_MSG, pull, coro);

			auto sqlres = (NetMsg *)ackmsg->m_data;
			PARSEPB_IF_FALSE(LPMsg::PBSqlParam, sqlres)
			{
			}
			if (!pbMsg.ret())
			{
				LP_ERROR << "Update Account roomid Fail";
				room = NULL;
			}
		}
		SendRoomInfo(account, client,room,pull,coro);
	}
	//ÊÍ·ÅËø
	m_transModule->SendToServer(ser, N_LOGIN_UNLOCK, msg);
	RemoveClient(account->name);
	return true;
}

void LoginModule::SendRoomInfo(SHARE<AccoutInfo>& account, SHARE<ClientObj>& client, RoomServer* room, c_pull & pull, SHARE<BaseCoro>& coro)
{
	if (!room)
	{
		LP_ERROR << " ERROR No Room Server Open";
		return;
	}

	LPMsg::RoomReadyInfo reqmsg;
	reqmsg.set_pid(account->id);
	reqmsg.set_roleid(account->roleId);

	auto& roompath = m_roomModule->GetRoomPath(room->id);
	auto ackmsg = m_transModule->RequestServerAsynMsg(roompath, N_ROOM_READY_TAKE_PLAYER, reqmsg, pull, coro);

	LPMsg::RoomInfo msg;
	msg.set_ip(room->ip);
	msg.set_port(room->port);
	msg.set_pid(account->id);
	m_netModule->SendNetMsg(client->sock, LPMsg::N_ACK_LOGIN_RES, msg);
}

void LoginModule::RemoveClient(const std::string & account)
{
	auto it = m_tmpClient.find(account);
	if (it == m_tmpClient.end())
		return;
	m_netModule->CloseNetObject(it->second->sock);
	m_tmpClient.erase(it);
}
