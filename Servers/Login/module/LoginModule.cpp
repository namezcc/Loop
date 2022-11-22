#include "LoginModule.h"
#include "NetObjectModule.h"
#include "MsgModule.h"
#include "Crypto/crchash.h"
//#include "GameReflectData.h"
#include "MysqlModule.h"
#include "SendProxyDbModule.h"
#include "RedisModule.h"
#include "RoomStateModule.h"
#include "TransMsgModule.h"
#include "EventModule.h"
#include "ScheduleModule.h"

#include "protoPB/base/LPSql.pb.h"
#include "protoPB/client/client.pb.h"
#include "protoPB/client/define.pb.h"
#include "protoPB/server/server.pb.h"
#include "protoPB/server/dbdata.pb.h"
#include "protoPB/server/server_msgid.pb.h"
#include "protoPB/server/proto_common.pb.h"

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
	m_redisModule = GET_MODULE(RedisModule);
	m_roomModule = GET_MODULE(RoomStateModule);
	m_transModule = GET_MODULE(TransMsgModule);
	m_eventModule = GET_MODULE(EventModule);
	m_schedule = GET_MODULE(ScheduleModule);


	m_msgModule->AddAsynMsgCall(LPMsg::CM_LOGIN,BIND_ASYN_CALL(OnClientLogin));

	//m_msgModule->AddMsgCall(501, BIND_NETMSG(OnTestPing));
	//m_msgModule->AddAsynMsgCall(500, BIND_ASYN_NETMSG(onTestAsyncPing));

	m_eventModule->AddEventCall(E_SOCKEK_CONNECT,BIND_EVENT(OnClientConnect,int32_t));


	m_lock_server.type = SERVER_TYPE::LOOP_LOGIN_LOCK;
	m_lock_server.serid = 0;

	m_room_mgr.type = SERVER_TYPE::LOOP_ROOM_MANAGER;
	m_room_mgr.serid = 0;
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
		LP_WARN << "allready wait to login account:" << pbMsg.account();
		if(itc->second.sock != msg->socket)
			m_netModule->CloseNetObject(msg->socket);
		return;
	}

	ClientObj client = {};
	client.sock = msg->socket;
	m_tmpClient[pbMsg.account()] = client;

	ExitCall failcall([this,&coro,&pbMsg]() {
		if (!coro->IsFail())
			return;
		RemoveClient(pbMsg.account());
	});

	LPMsg::VString lockpb;
	lockpb.set_val(pbMsg.account());

	auto acklock = m_transModule->RequestServerAsynMsg(m_lock_server, LPMsg::IM_ACCOUNT_GET_UID, lockpb, pull, coro);
	PARSEPB_NAME_IF_FALSE(ackpb,LPMsg::Int32Value, acklock)
	{
		return coro->SetFail();
	}
	if (ackpb.value() == 0)
	{
		coro->SetFail();
		LP_WARN << "login get accont fail:" << pbMsg.account();
		return;
	}

	auto ackgate = m_transModule->RequestServerAsynMsg(m_room_mgr, LPMsg::IM_RMGR_PLAYER_LOGIN, ackpb, pull, coro);
	{
		PARSEPB_NAME_IF_FALSE(ackpb, LPMsg::RoomInfo, ackgate)
		{
			LP_ERROR << "login get gate info error ";
			return coro->SetFail();
		}
		if (ackpb.uid() == 0)
		{
			LP_WARN << "no room info get " << pbMsg.account();
		}
		m_netModule->SendNetMsg(msg->socket, LPMsg::SM_LOGIN_RES, ackpb);
		RemoveClient(pbMsg.account());
	}
}

void LoginModule::RemoveClient(const std::string & account)
{
	auto it = m_tmpClient.find(account);
	if (it == m_tmpClient.end())
		return;
	m_netModule->CloseNetObject(it->second.sock);
	m_tmpClient.erase(it);
}

int32_t LoginModule::getRoomFromRedis(int64_t pid)
{
	auto key = "login:" + Loop::Cvto<std::string>(pid);
	std::map<std::string, std::string> res;
	if (!m_redisModule->HGetAll(key, res))
		return 0;

	if (res.empty())
		return 0;

	auto lastroom = Loop::Cvto<int32_t>(res["room"]);
	if (lastroom == 0)
		return 0;

	auto lastlogin = Loop::Cvto<int32_t>(res["last_login"]);
	auto lastlogout = Loop::Cvto<int32_t>(res["last_logout"]);

	if (lastlogin > lastlogout)	//还在线
		return lastroom;
	else
		return 0;				//下线
}

void LoginModule::saveLoginToRedis(int64_t pid, int32_t roomid)
{
	auto key = "login:" + Loop::Cvto<std::string>(pid);
	std::map<std::string, std::string> res;

	res["last_login"] = Loop::to_string(Loop::GetSecend());
	res["room"] = Loop::to_string(roomid);

	m_redisModule->HMSet(key, res);
}

