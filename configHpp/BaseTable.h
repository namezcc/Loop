#ifndef BASE_TABLE_H
#define BASE_TABLE_H

#include <string>

using namespace std;

struct BaseTable;
typedef BaseTable* (*TableCreat)(int);

struct BaseTable
{	
	static BaseTable* CreateTable(string name,int key)
	{
		auto it = m_createMap.find(name);
		if (it == m_createMap.end())
			return nullptr;
		return it->second(key);
	}

	static void Regist(string name,TableCreat func)
	{
		m_createMap[name] = func;
	}
private:
	static map<string, TableCreat> m_createMap;
};

struct TableRegist
{
	TableRegist(string name, TableCreat func)
	{
		BaseTable::Regist(name, func);
	}
};

#endif