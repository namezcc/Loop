#ifndef REFLECT_DATA_H
#define REFLECT_DATA_H
#include "Reflection.h"
struct Admin
{
	string name;
	string pass;
	int level;
};

REFLECT(Admin,name,pass,level)
PARAMKEY(Admin,name)

#endif