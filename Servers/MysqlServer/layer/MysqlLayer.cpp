#include "MysqlLayer.h"
#include "MysqlModule.h"
#include "MsgModule.h"
#include "ExecuteMysqlModule.h"

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