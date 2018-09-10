#ifndef GAME_TABLE_MODULE_H
#define GAME_TABLE_MODULE_H

#include "BaseModule.h"

class MysqlModule;

class GameTableModule:public BaseModule
{
public:
	GameTableModule(BaseLayer* l);
	~GameTableModule();

	void CreateTables(const int32_t& group);
protected:
	virtual void Init() override;

	int64_t GetUid(const int32_t& group);
	void CreateAccount(const int32_t& group);
	void CreatePlayer(const int32_t& group);

private:
	MysqlModule * m_mysqlModule;

};

#endif
