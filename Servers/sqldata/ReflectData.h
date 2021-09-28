#ifndef REFLECT_DATA_H
#define REFLECT_DATA_H
#include "Reflection.h"
struct Admin
{
	string user;
	string pass;
	int level;
};
REFLECT(Admin,user,pass,level)

TABLE_DESC_BEGAN(Admin,user)
FIELD_DESC(user,SQL_FIELD_TYPE::SQL_VARCHAR,64,false,"",false,"")
FIELD_DESC(pass, SQL_FIELD_TYPE::SQL_VARCHAR, 64, false, "", false, "")
FIELD_DESC(level, SQL_FIELD_TYPE::SQL_INT, 11, false, "", false, "")
TABLE_DESC_END

struct ServerInfo
{
	int id;
	string ip;
	int port;
	string name;
	int status;
};
REFLECT(ServerInfo, id, ip, port, name)

TABLE_DESC_BEGAN(ServerInfo, id)
FIELD_DESC(id, SQL_FIELD_TYPE::SQL_INT, 11, false, "", false, "")
FIELD_DESC(ip, SQL_FIELD_TYPE::SQL_VARCHAR,64, false, "", false, "'server ip'")
FIELD_DESC(port, SQL_FIELD_TYPE::SQL_INT, 11, false, "", false, "'server port'")
FIELD_DESC(name, SQL_FIELD_TYPE::SQL_VARCHAR, 64, false, "", false, "'server name'")
TABLE_DESC_END

struct dbMachine
{
	int32_t id;
	std::string ip;
};
REFLECT(dbMachine, id, ip)

TABLE_DESC_BEGAN(dbMachine, id)
FIELD_DESC(id, SQL_FIELD_TYPE::SQL_INT, 11, false, "", false, "")
FIELD_DESC(ip, SQL_FIELD_TYPE::SQL_VARCHAR, 64, false, "", false, "")
TABLE_DESC_END

struct MachineServer
{
	int32_t type;
	int32_t id;
	int32_t machine;
	int32_t port;
	int32_t state;
};

REFLECT(MachineServer, type, id, machine, port)

TABLE_DESC_BEGAN(MachineServer, type,id)
FIELD_DESC(type, SQL_FIELD_TYPE::SQL_INT, 11, false, "", false, "")
FIELD_DESC(id, SQL_FIELD_TYPE::SQL_INT, 11, false, "", false, "'server id'")
FIELD_DESC(machine, SQL_FIELD_TYPE::SQL_INT, 11, false, "", false, "")
FIELD_DESC(port, SQL_FIELD_TYPE::SQL_INT, 11, false, "", false, "")
TABLE_DESC_END

#endif