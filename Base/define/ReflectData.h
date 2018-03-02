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
FIELD_DESC(ip, SQL_FIELD_TYPE::SQL_VARCHAR,64, false, "", false, "server ip")
FIELD_DESC(port, SQL_FIELD_TYPE::SQL_INT, 11, false, "", false, "server port")
FIELD_DESC(name, SQL_FIELD_TYPE::SQL_VARCHAR, 64, false, "", false, "server name")
TABLE_DESC_END

#endif