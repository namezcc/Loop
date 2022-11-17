#include "LoginLockModule.h"
#include "MsgModule.h"
#include "NetObjectModule.h"
#include "ScheduleModule.h"
#include "MysqlModule.h"
#include "TransMsgModule.h"
#include "help_function.h"
#include "proto_sql.h"

#include <protobuf/google/protobuf/repeated_field.h>
#include "protoPB/server/server.pb.h"
#include "protoPB/server/server_msgid.pb.h"
#include "protoPB/server/proto_common.pb.h"
#include "protoPB/server/dbdata.pb.h"

#define SQL_BUFF m_sql_buff, sizeof(m_sql_buff)

LoginLockModule::LoginLockModule(BaseLayer * l):BaseModule(l)
{
	m_now_stamp = 0;
	m_add_num = 0;
}

LoginLockModule::~LoginLockModule()
{
}

void LoginLockModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);
	m_netObjModule = GET_MODULE(NetObjectModule);
	m_schedulModule = GET_MODULE(ScheduleModule);
	m_mysql_module = GET_MODULE(MysqlModule);
	m_trans_mod = GET_MODULE(TransMsgModule);

	m_msgModule->AddMsgCall(LPMsg::IM_ACCOUNT_GET_UID, BIND_SHARE_CALL(onPlayerLogin));
	
	m_msgModule->AddMsgCallBack(N_LOGIN_LOCK, this, &LoginLockModule::OnLoginLock);
	m_msgModule->AddMsgCallBack(N_LOGIN_UNLOCK, this, &LoginLockModule::OnLoginUnlock);

	m_msgModule->AddMsgCallBack(N_GET_DBINDEX, this, &LoginLockModule::onGetDbIndex);
	m_msgModule->AddMsgCallBack(N_ADD_DBINDEX_NUM, this, &LoginLockModule::onDbIndexAddNum);

	m_msgModule->AddMsgCall(501, BIND_NETMSG(onNormalTest));
	m_msgModule->AddAsynMsgCall(500, BIND_ASYN_NETMSG(onAsyncTest));

	m_schedulModule->AddTimePointTask(BIND_TIME(CheckOutTime), -1, 0);
	//m_schedulModule->AddInterValTask(BIND_TIME(showTestNum), 5000, -1,3000);

	m_pp_num = 0;
	m_start_time = 0;
}

void LoginLockModule::AfterInit()
{
	loadDbPlayerNum();
	m_now_stamp = Loop::GetSecend();
}

void LoginLockModule::onPlayerLogin(SHARE<BaseMsg>& msg)
{
	auto netmsg = (NetMsg*)msg->m_data;
	TRY_PARSEPB(LPMsg::VString, netmsg);
	LPMsg::Int32Value pb;
	auto it = m_account_info.find(pbMsg.val());
	if (it != m_account_info.end())
	{
		pb.set_value(it->second);
	}
	else {
		auto uid = genPlayerUid(pbMsg.val());
		pb.set_value(uid);
	}
	m_netObjModule->ResponseMsg(netmsg->socket, msg, pb);
}

void LoginLockModule::OnLoginLock(SHARE<BaseMsg>& msg)
{
	auto netmsg = (NetMsg*)msg->m_data;
	TRY_PARSEPB(LPMsg::LoginLock, netmsg);

	auto it = m_lockPid.find(pbMsg.pid());
	auto now = Loop::GetSecend();
	if (it == m_lockPid.end() || it->second < now)
	{
		m_netObjModule->ResponseMsg(netmsg->socket, msg, pbMsg);
		m_lockPid[pbMsg.pid()] = now + LOGIN_LOCK_OUT_TIME;
	}
	else
	{
		pbMsg.set_pid(0);
		m_netObjModule->ResponseMsg(netmsg->socket, msg, pbMsg);
	}
}

void LoginLockModule::OnLoginUnlock(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::LoginLock, msg);
	m_lockPid.erase(pbMsg.pid());
}

void LoginLockModule::onGetDbIndex(SHARE<BaseMsg>& msg)
{
	auto netmsg = (NetMsg*)msg->m_data;

	auto spack = GET_LAYER_MSG(BuffBlock);
	auto idx = getDbIndex();
	if (idx == 0)
	{
		LP_ERROR << "no db for player";
		return;
	}

	if (Loop::GetSecend() > m_now_stamp)
	{
		m_now_stamp = Loop::GetSecend();
		m_add_num = 0;
	}

	m_add_num++;
	if (m_add_num >= 0xFF)
	{
		m_add_num = 0;
		m_now_stamp++;
	}

	auto uid = createPlayerUid(idx, m_now_stamp, m_add_num);

	spack->writeInt32(idx);
	spack->writeInt64(uid);

	m_netObjModule->ResponseMsg(netmsg->socket, msg, spack);
}

void LoginLockModule::onDbIndexAddNum(NetMsg * msg)
{
	auto pack = msg->m_buff;
	auto id = pack->readInt32();

	auto it = m_id_index.find(id);
	if (it == m_id_index.end() || it->second < 0 || it->second >= m_db_player_num.size())
		return;

	m_db_player_num[it->second].second++;

	char _sql[1024];
	sprintf_s(_sql,sizeof(_sql), "UPDATE `db_player_num` SET `count`=%d WHERE `dbid`=%d;", m_db_player_num[it->second].second, m_db_player_num[it->second].first);	

	m_mysql_module->Query(_sql);

	if (m_db_player_num[it->second].second >= MAX_PLAYER_NUM_PER_DB)
	{
		if (m_db_player_num.size() == 1)
		{
			m_db_player_num.clear();
			m_id_index.clear();
			return;
		}

		auto oldn = m_db_player_num[it->second];
		m_db_player_num[it->second] = m_db_player_num[m_db_player_num.size()-1];
		m_db_player_num[m_db_player_num.size() - 1] = oldn;

		//std::swap(m_db_player_num.begin() + it->second, m_db_player_num.end() - 1);

		m_id_index[m_db_player_num[it->second].first] = it->second;
		m_id_index[m_db_player_num.cbegin()->first] = m_db_player_num.size() - 1;
	}
}

void LoginLockModule::onNormalTest(NetMsg* msg)
{
	TRY_PARSEPB(LPMsg::LoginLock, msg);

	m_pp_num++;
	ServerNode ser{ SERVER_TYPE::LOOP_LOGIN,1 };
	m_trans_mod->SendToServer(ser, 501, pbMsg);
}

void LoginLockModule::onAsyncTest(NetMsg* msg, c_pull & pull, SHARE<BaseCoro>& coro)
{
	TRY_PARSEPB(LPMsg::LoginLock, msg);
	m_pp_num++;

	ServerNode ser{ SERVER_TYPE::LOOP_LOGIN,1 };
	auto ack = m_trans_mod->ResponseServerAsynMsg(ser, pull.get(), pbMsg, pull, coro);

	TRY_PARSEPB_NAME(pback,LPMsg::LoginLock, ack);

	m_pp_num++;
	m_trans_mod->SendToServer(ser, 500, pback);
}

void LoginLockModule::CheckOutTime(int64_t & dt)
{
	auto now = dt / 1000;
	for (auto it = m_lockPid.begin();it!= m_lockPid.end();)
	{
		if (it->second > now)
			m_lockPid.erase(it++);
		else
			it++;
	}
}

void LoginLockModule::showTestNum(int64_t & dt)
{
	if (m_start_time == 0)
	{
		m_start_time = dt;

		m_schedulModule->AddInterValTask([this](int64_t& tt) {
		
			/*m_msgModule->DoCoroFunc([this](c_pull& pull, SHARE<BaseCoro>& coro) {
				ServerNode ser{SERVER_TYPE::LOOP_LOGIN,1};
				ServerPath path;
				path.push_back(*GetLayer()->GetServer());
				path.push_back(ser);
				m_trans_mod->RequestServerAsynMsg(path, 500, LPMsg::LoginLock{},pull,coro);
			});*/
			m_trans_mod->SendToServer(ServerNode{ SERVER_TYPE::LOOP_LOGIN,1 }, 501, LPMsg::LoginLock{});
		}, 50, 1000);
	}
	else {
		auto pernum = m_pp_num * 1000 / 5000;
		LP_INFO << "ping poing per second " << pernum;
		m_pp_num = 0;
	}
}

void LoginLockModule::loadDbPlayerNum()
{
	gopb::RepeatedPtrField<LPMsg::DB_player_num_info> repplayer;
	select_db_player_num_info(repplayer, m_mysql_module, SQL_BUFF);
	if (repplayer.size() == 0)
	{
		LP_ERROR << "player num info error size 0";
		getLoopServer()->closeServer();
		return;
	}

	for (auto i:repplayer)
	{
		if (i.num() < i.maxnum() || i.maxnum() <= 0)
			continue;
		m_db_player_num_info.push_back(i);
	}

	gopb::RepeatedPtrField<LPMsg::DB_account> repaccount;
	select_account(repaccount, m_mysql_module, SQL_BUFF);
	for (auto& i:repaccount)
	{
		m_account_info[i.platform_uid()] = i.game_uid();
		m_uid_check.insert(i.game_uid());
	}
}

int32_t LoginLockModule::getDbIndex()
{
	if (m_db_player_num.empty())
		return 0;

	auto r = rand() % m_db_player_num.size();
	return m_db_player_num[r].first;
}

int32_t LoginLockModule::genPlayerUid(const std::string& uuid)
{
	if (m_db_player_num_info.empty())
		return 0;

	while (m_db_player_num_info.size() > 0)
	{
		auto r = rand() % m_db_player_num_info.size();
		auto& info = m_db_player_num_info[r];

		while (true)
		{
			auto uid = createPlayerUid32(info.dbid(), info.num());
			if (uid <= 0)
			{
				LP_ERROR << "create uid error dbid:" << info.dbid() << "num:" << info.num();
				m_db_player_num_info.erase(m_db_player_num_info.begin()+r);
				break;
			}

			if (m_uid_check.find(uid) != m_uid_check.end())
			{
				info.set_num(info.num() + 1);
				if (info.num() >= info.maxnum())
				{
					m_db_player_num_info.erase(m_db_player_num_info.begin() + r);
					break;
				}
				continue;
			}

			LPMsg::DB_account pb;
			pb.set_create_time(Loop::GetSecend());
			pb.set_game_uid(uid);
			pb.set_platform_uid(uuid);
			if (!insert_account(pb, m_mysql_module, SQL_BUFF))
				return 0;

			info.set_num(info.num() + 1);
			update_db_player_num_info(info, m_mysql_module, SQL_BUFF);
			if (info.num() >= info.maxnum())
				m_db_player_num_info.erase(m_db_player_num_info.begin() + r);
			return uid;
		}
	}
	return 0;
}
