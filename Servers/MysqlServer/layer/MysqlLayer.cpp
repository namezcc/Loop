#include "MysqlLayer.h"
#include "MysqlModule.h"
#include "MsgModule.h"
#include "ExecuteMysqlModule.h"
#include "Coro2Module.h"

MysqlLayer::MysqlLayer():BaseLayer(LY_MYSQL)
{
}

MysqlLayer::~MysqlLayer()
{
}

void MysqlLayer::init()
{
	CreateModule<MysqlModule>();
	CreateModule<MsgModule>();
	CreateModule<ExecuteMysqlModule>();
	CreateModule<Coro2Module>();
}

void MysqlLayer::loop()
{
}

void MysqlLayer::close()
{
}

void MysqlLayer::GetDefaultTrans(int & ltype, int & lid)
{
	ltype = LY_LOGIC;
	lid = 0;
}