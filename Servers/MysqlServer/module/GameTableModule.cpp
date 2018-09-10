#include "GameTableModule.h"
#include "MysqlModule.h"

#include "GameReflectData.h"
#include "ConfigObjects.h"

#define INIT_TABLE(T,group)	\
	auto tname = Reflect<T>::Name() + std::to_string(group); \
	m_mysqlModule->InitTable<T>(tname);

GameTableModule::GameTableModule(BaseLayer * l):BaseModule(l)
{
}

GameTableModule::~GameTableModule()
{
}

void GameTableModule::Init()
{
	m_mysqlModule = GET_MODULE(MysqlModule);
}

void GameTableModule::CreateTables(const int32_t & group)
{
	CreateAccount(group);
	CreatePlayer(group);
}

int64_t GameTableModule::GetUid(const int32_t & group)
{
	auto dbgroup = m_mysqlModule->GetDBGroup();
	int64_t uid = (dbgroup << 8) | group;
	uid <<= 48;
	return uid;
}

void GameTableModule::CreateAccount(const int32_t & group)
{
	int64_t uid = GetUid(group);

	INIT_TABLE(AccoutInfo, group)
	//还要插入默认数据
	AccoutInfo data1;

	Reflect<AccoutInfo> rf(&data1);
	rf.Set_id(uid);

	if (m_mysqlModule->Select(rf, tname))
		return;

	rf.Set_name("_%Template%_");
	rf.Set_pass("%123456789%");
	m_mysqlModule->Insert(rf, tname);
}

void GameTableModule::CreatePlayer(const int32_t & group)
{
	int64_t uid = GetUid(group);

	INIT_TABLE(Player, group);
	Player _data;
	_data.Set_id(uid);
	if (m_mysqlModule->Select(*_data.m_sql, tname))
		return;
	_data.Set_name("_%Template%_");
	m_mysqlModule->Insert(*_data.m_sql, tname);
}
