#include "MysqlManagerModule.h"
#include "GameReflectData.h"
#include "MysqlModule.h"
#include "MsgModule.h"
#include "TransMsgModule.h"


MysqlManagerModule::MysqlManagerModule(BaseLayer * l):BaseModule(l)
{

}

MysqlManagerModule::~MysqlManagerModule()
{
}

void MysqlManagerModule::Init()
{
	m_mysqlmodule = GET_MODULE(MysqlModule);
	m_msgmodule = GET_MODULE(MsgModule);
	m_transModule = GET_MODULE(TransMsgModule);

	
	m_msgmodule->AddMsgCallBack(N_GET_MYSQL_GROUP, this, &MysqlManagerModule::OnGetGroupId);
	m_msgmodule->AddMsgCallBack(N_MYSQL_MSG, this, &MysqlManagerModule::OnGetMysqlMsg);
	m_msgmodule->AddMsgCallBack(L_MYSQL_MSG, this, &MysqlManagerModule::OnGetMysqlRes);
	m_msgmodule->AddMsgCallBack(N_UPDATE_TABLE_GROUP, this, &MysqlManagerModule::OnUpdateTableGroup);
	m_msgmodule->AddMsgCallBack(N_ADD_TABLE_GROUP, this, &MysqlManagerModule::OnAddTableGroup);
	m_msgmodule->AddMsgCallBack(N_CREATE_ACCOUNT, this, &MysqlManagerModule::OnCreateAccount);
	

	m_index = 0;
	auto it = GetLayer()->GetPipes().find(LY_MYSQL);
	m_sqlLayerNum = it->second.size();
}

void MysqlManagerModule::AfterInit()
{
}

void MysqlManagerModule::BeforExecute()
{
	CreateMysqlTable();
	InitTableGroupNum();
}

void MysqlManagerModule::InitTableGroupNum()
{
	MultRow row;
	SqlRow field;
	m_mysqlmodule->Select("show TABLES;", row, field);
	m_tableGroup = 0;
	for (auto& r:row)
		for (auto& t:r)
			if (t.find(Reflect<AccoutInfo>::Name()))
				++m_tableGroup;

	for (size_t i = 0; i < m_sqlLayerNum; i++)
	{//拿 NetSocket 结构代用一下
		auto num = GET_LAYER_MSG(NetSocket);
		num->socket = m_tableGroup;
		m_msgmodule->SendMsg(LY_MYSQL, i, L_UPDATE_TABLE_GROUP, num);
	}

	for (size_t i = 2; i <= m_tableGroup; i++)
		CreateMysqlTable(i);
}

void MysqlManagerModule::CreateMysqlTable(int group)
{
	auto dbgroup = m_mysqlmodule->GetDBGroup();
	int64_t uid = (dbgroup << 8) | group;
	uid <<= 48;

	auto tname = Reflect<AccoutInfo>::Name() + std::to_string(group);
	m_mysqlmodule->InitTable<AccoutInfo>(tname);
	//还要插入默认数据
	AccoutInfo data1;

	Reflect<AccoutInfo> rf(&data1);
	rf.Set_id(uid);
	rf.Set_name("_%Template%_");
	rf.Set_pass("%123456789%");
	m_mysqlmodule->Insert(rf, tname);
}

void MysqlManagerModule::OnGetGroupId(NetServerMsg * msg)
{
	TRY_PARSEPB(LPMsg::EmptyPB,msg,m_msgmodule);
	if (msg->path.size() <= 0)
	{
		LP_ERROR(m_msgmodule) << "Get group Error Path size = 0";
		return;
	}
	
	LPMsg::UpdateTableGroup xmsg;
	xmsg.set_groupcount(m_mysqlmodule->GetDBGroup());
	m_transModule->SendBackServer(msg->path, N_GET_MYSQL_GROUP, xmsg);
}

void MysqlManagerModule::OnCreateAccount(NetServerMsg * msg)
{
	if (msg->path.size() == 0)
	{
		LP_ERROR(m_msgmodule) << "ERROR Server path";
		return;
	}

	auto reply = GetLayer()->GetSharedLoop<SqlReply>();
	if (!reply->pbMsg.ParseFromArray(msg->getCombinBuff(GetLayer())->m_buff, msg->getLen()))
	{
		LP_ERROR(m_msgmodule) << "parse PBSqlParam error";
		return;
	}
	
	if (reply->pbMsg.uid() == 0)
		return;
	
	//hash 到组
	auto gid = reply->pbMsg.uid()%(m_tableGroup*2)+1;
	if (gid > m_tableGroup)
		gid = m_tableGroup;

	reply->path = move(msg->path);
	SendSqlReply(reply, gid);
}

void MysqlManagerModule::OnGetMysqlMsg(NetServerMsg * msg)
{
	if (msg->path.size() == 0)
	{
		LP_ERROR(m_msgmodule) << "ERROR Server path";
		return;
	}

	auto reply = GetLayer()->GetSharedLoop<SqlReply>();
	if (!reply->pbMsg.ParseFromArray(msg->getCombinBuff(GetLayer())->m_buff, msg->getLen()))
	{
		LP_ERROR(m_msgmodule) << "parse PBSqlParam error";
		return;
	}
	//检查是否有组ID 高 16位 8 数据库 id 8 组id
	auto gid = (reply->pbMsg.uid() >> 48) & 0xff;
	if (gid == 0)
		return;

	reply->path = move(msg->path);
	SendSqlReply(reply, gid);
}

void MysqlManagerModule::OnGetMysqlRes(LMsgSqlParam * msg)
{
	auto it = m_replay.find(msg->index);
	if (it == m_replay.end())
		return;

	auto reply = it->second;
	
	reply->pbMsg.set_ret(msg->param->ret);

	if (reply->pbMsg.field_size() != msg->param->field.size())
	{
		reply->pbMsg.clear_field();
		for (auto& s : msg->param->field)
			reply->pbMsg.add_field(s);
	}
	
	if (reply->pbMsg.value_size() != msg->param->value.size())
	{
		reply->pbMsg.clear_value();
		for (auto& s : msg->param->value)
			reply->pbMsg.add_value(s);
	}
	//send back
	m_transModule->SendBackServer(reply->path, reply->pbMsg.reply(), reply->pbMsg);

	m_replay.erase(it);
}

void MysqlManagerModule::OnUpdateTableGroup(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::UpdateTableGroup, msg, m_msgmodule);
	m_tableGroup = pbMsg.groupcount();

	for (size_t i = 0; i < m_sqlLayerNum; i++)
	{//拿 NetSocket 结构代用一下
		auto num = GET_LAYER_MSG(NetSocket);
		num->socket = m_tableGroup;
		m_msgmodule->SendMsg(LY_MYSQL, i, L_UPDATE_TABLE_GROUP, num);
	}
}

void MysqlManagerModule::OnAddTableGroup(NetMsg * msg)
{
	TRY_PARSEPB(LPMsg::UpdateTableGroup, msg, m_msgmodule);

	++m_tableGroup;
	CreateMysqlTable(m_tableGroup);
	pbMsg.set_groupcount(m_tableGroup);
	//向本组server 通知更新
	m_transModule->SendToAllServer(LOOP_MYSQL, N_UPDATE_TABLE_GROUP, pbMsg);
}

void MysqlManagerModule::SendSqlReply(SHARE<SqlReply>& reply,const int& gid)
{
	auto lmsg = GET_LAYER_MSG(LMsgSqlParam);
	lmsg->param = GET_LAYER_MSG(SqlParam);

	lmsg->param->opt = reply->pbMsg.opt();
	lmsg->param->tab = reply->pbMsg.table() + std::to_string(gid);
	lmsg->param->kname.assign(reply->pbMsg.kname().begin(), reply->pbMsg.kname().end());
	lmsg->param->kval.assign(reply->pbMsg.kval().begin(), reply->pbMsg.kval().end());
	lmsg->param->field.assign(reply->pbMsg.field().begin(), reply->pbMsg.field().end());
	lmsg->param->value.assign(reply->pbMsg.value().begin(), reply->pbMsg.value().end());

	auto sendid = GetSendLayerId();

	if (reply->pbMsg.reply()>0)
	{
		lmsg->index = m_index;
		m_replay[m_index] = reply;
	}
	m_msgmodule->SendMsg(LY_MYSQL, sendid, L_MYSQL_MSG, lmsg);
}

int MysqlManagerModule::GetSendLayerId()
{
	++m_index;
	return m_index%m_sqlLayerNum;
}