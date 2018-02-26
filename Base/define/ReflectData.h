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
PARAMKEY(Admin, user)

struct ServerInfo
{
	int id;
	string ip;
	int post;
	string name;
	int status;
};
REFLECT(ServerInfo, id, ip, post, name)
PARAMKEY(ServerInfo, id)

#endif