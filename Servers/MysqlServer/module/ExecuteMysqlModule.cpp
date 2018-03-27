#include "ExecuteMysqlModule.h"
#include "MsgModule.h"
#include "MysqlModule.h"

ExecuteMysqlModule::ExecuteMysqlModule(BaseLayer * l):BaseModule(l)
{
}

ExecuteMysqlModule::~ExecuteMysqlModule()
{
}

void ExecuteMysqlModule::Init()
{
	m_msgModule = GET_MODULE(MsgModule);
	m_mysqlModule = GET_MODULE(MysqlModule);

	m_msgModule->AddMsgCallBack<LMsgSqlParam>(L_MYSQL_MSG, this, &ExecuteMysqlModule::OnGetMysqlMsg);
	m_msgModule->AddMsgCallBack<NetSocket>(L_UPDATE_TABLE_GROUP, this, &ExecuteMysqlModule::OnUpdateTableGroup);

}

void ExecuteMysqlModule::AfterInit()
{
}

void ExecuteMysqlModule::OnGetMysqlMsg(LMsgSqlParam * msg)
{
	switch (msg->param->opt)
	{
	case SQL_SELECT:
		msg->param->ret = m_mysqlModule->Select(*msg->param);
		break;
	case SQL_INSERT:
		msg->param->ret = m_mysqlModule->Insert(*msg->param);
		break;
	case SQL_DELETE:
		msg->param->ret = m_mysqlModule->Delete(*msg->param);
		break;
	case SQL_UPDATE:
		msg->param->ret = m_mysqlModule->Update(*msg->param);
		break;
	case SQL_INSERT_SELECT:
		msg->param->ret = false;
		if (m_mysqlModule->Insert(*msg->param))
		{
			msg->param->field.clear();
			msg->param->value.clear();
			msg->param->ret = m_mysqlModule->Select(*msg->param);
		}
		break;
	}

	if (msg->index > 0)
	{
		auto lmsg = new LMsgSqlParam();
		lmsg->index = msg->index;
		std::swap(lmsg->param, msg->param);
		m_msgModule->SendMsg(LY_LOGIC, 0, L_MYSQL_MSG, lmsg);
	}
}

void ExecuteMysqlModule::OnUpdateTableGroup(NetSocket * num)
{
	m_tableGroup = num->socket;
}