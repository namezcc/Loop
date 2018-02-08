#include "MysqlModule.h"
#include <mysqlpp/include/mysql++.h>
#include "MsgModule.h"

#define MYSQL_TRY try{

/*
std::cout << "BadQuery [" << msg << "] Error: " << er.what() << std::endl; \
std::cout << "BadConversion [" << msg << "] Error:" << er.what() << " retrieved data size:" << 
er.retrieved << ", actual size:" << er.actual_size << std::endl; \
std::cerr << "Failed to connect to database server: " <<er.what()  << std::endl; \
std::cout << "mysqlpp::Exception [" << msg << "] Error:" << er.what() << std::endl; \
std::cout << "std::exception [" <<msg << "] Error:Unknown " << std::endl; \
*/

#define MYSQL_CATCH(msg) }\
	catch (mysqlpp::BadQuery er) \
    { \
        LP_ERROR(m_msgModule)<<"BadQuery ["<<msg<<"] Error: "<<er.what(); \
        return false; \
    } \
    catch (const mysqlpp::BadConversion& er)  \
    { \
		LP_ERROR(m_msgModule)<<"BadConversion ["<<msg<<"] Error:"<<er.what()<<" retrieved data size:"<<er.retrieved<<", actual size:"<<er.actual_size;	\
        return false; \
    } \
	catch (const mysqlpp::ConnectionFailed& er) \
	{ \
		LP_ERROR(m_msgModule)<<"Failed to connect to database server: "<<er.what();	\
		return false; \
	} \
    catch (const mysqlpp::Exception& er) \
    { \
		LP_ERROR(m_msgModule)<<"mysqlpp::Exception ["<<msg<<"] Error:"<<er.what();	\
        return false; \
    }\
    catch ( ... ) \
    { \
		LP_ERROR(m_msgModule)<<"std::exception ["<<msg<<"] Error:Unknown ";	\
        return false; \
    }

void SqlParam::init(FactorManager * fm)
{
}

void SqlParam::recycle(FactorManager * fm)
{
	kname.clear();
	kval.clear();
	field.clear();
	value.clear();
}


MysqlModule::MysqlModule(BaseLayer * l):BaseModule(l)
{
	m_sqlConn.reset(new mysqlpp::Connection(false));
}

MysqlModule::~MysqlModule()
{
}

void MysqlModule::Init()
{
	m_msgModule = GetLayer()->GetModule<MsgModule>();
}

void MysqlModule::AfterInit()
{
	if (!m_sqlConn->connected())
		Reconnect();
}

void MysqlModule::Execute()
{
}

void MysqlModule::SetConnect(const string & dbname, const string & ip, const string & user, const string & pass, const int & port)
{
	m_dbname = dbname;
	m_ip = ip;
	m_user = user;
	m_pass = pass;
	m_port = port;
}

bool MysqlModule::Connect(const string& dbname, const string & ip, const string & user, const string & pass, const int & port)
{
	m_dbname = dbname;
	m_ip = ip;
	m_user = user;
	m_pass = pass;
	m_port = port;

	m_sqlConn->set_option(new mysqlpp::MultiStatementsOption(true));
	m_sqlConn->set_option(new mysqlpp::SetCharsetNameOption("utf8mb4"));
	m_sqlConn->set_option(new mysqlpp::ReconnectOption(true));
	m_sqlConn->set_option(new mysqlpp::ConnectTimeoutOption(60));

	return m_sqlConn->connect(dbname.data(), ip.data(), user.data(), pass.data(), port);
}

bool MysqlModule::Reconnect()
{
	if (m_sqlConn->connected())
		return true;
	return m_sqlConn->connect(m_dbname.data(), m_ip.data(), m_user.data(), m_pass.data(), m_port);
}

bool MysqlModule::Query(const string & str)
{
	bool ret = false;
	auto query = m_sqlConn->query(str);

	ExitCall _call([this,&ret,&str]() {
		if (!ret)
			LP_ERROR(m_msgModule)<< "Query Error:"<< str;
		//cout << "Query Error:" << str <<endl;
	});
	MYSQL_TRY
		query.execute();
	MYSQL_CATCH("")
	return ret = true;
}

bool MysqlModule::Select(const string & str, MultRow & res, SqlRow & files)
{
	bool ret = false;
	auto query = m_sqlConn->query(str);

	ExitCall _call([this,&ret, &str]() {
		if (!ret)
			LP_ERROR(m_msgModule)<< "Select Error:"<< str;
		//cout << "Select Error:" << str << endl;
	});
	MYSQL_TRY
		auto result = query.store();
		if (result)
		{
			for (size_t i = 0; i < result.num_fields(); i++)
				files.push_back(result.field_name(i));

			for (size_t i = 0; i < result.num_rows(); i++)
			{
				SqlRow row;
				for (size_t j = 0; j < result.num_fields(); j++)
				{
					row.push_back(result[i][j].data());
				}
				res.push_back(std::move(row));
			}
		}
	MYSQL_CATCH("")
	return ret = true;
}

bool MysqlModule::Inster(SqlParam & p)
{
	bool ret = false;
	auto q = m_sqlConn->query();
	Qinsert(q, p.tab, p.field, p.value);
	ExitCall _call([this,&ret,&q]() {
		if (!ret)
			LP_ERROR(m_msgModule)<<"Inster Error:"<<q.str();
		//cout<<"Inster Error:" << q.str() << endl;
	});
	MYSQL_TRY
		q.execute();
	MYSQL_CATCH("")
	return ret=true;
}

bool MysqlModule::Delete(SqlParam & p)
{
	bool ret = false;
	auto q = m_sqlConn->query();
	q<<"DELETE FROM "<<p.tab;
	Qwhere(q, p.kname, p.kval);
	ExitCall _call([this,&ret, &q]() {
		if (!ret)
			LP_ERROR(m_msgModule)<< "Delete Error:"<< q.str();
		//cout << "Delete Error:" << q.str() << endl;
	});
	MYSQL_TRY
		q.execute();
	MYSQL_CATCH("")
	return ret = true;
}

bool MysqlModule::Update(SqlParam & p)
{
	bool ret = false;
	auto q = m_sqlConn->query();
	Qupdate(q, p.tab, p.field, p.value);
	Qwhere(q, p.kname, p.kval);
	ExitCall _call([this,&ret, &q]() {
		if (!ret)
			LP_ERROR(m_msgModule)<< "Update Error:"<<q.str();
		//cout << "Update Error:" << q.str() << endl;
	});
	MYSQL_TRY
		q.execute();
	MYSQL_CATCH("")
	return ret = true;
}

bool MysqlModule::Select(SqlParam & p)
{
	bool ret = false;
	auto q = m_sqlConn->query();
	Qselect(q, p.tab, p.field);
	Qwhere(q, p.kname, p.kval);
	ExitCall _call([this,&ret, &q]() {
		if (!ret)
			LP_ERROR(m_msgModule)<<"Select Error:"<< q.str();
		//cout << "Select Error:" << q.str() << endl;
	});
	MYSQL_TRY
		auto res = q.store();
		if (res)
		{
			for (size_t i = 0; i < res.num_fields(); i++)
				p.value.push_back(res[0][i].data());
		}
	MYSQL_CATCH("")
	return ret = true;
}

void MysqlModule::Qupdate(mysqlpp::Query & q, const string & tab, SqlRow & fields, SqlRow & vals)
{
	q << "UPDATE " << tab << " SET ";
	for (int i = 0; i < fields.size(); ++i)
	{
		if (i == 0)
			q << fields[i] << " = " << mysqlpp::quote << vals[i];
		else
			q << "," << fields[i] << " = " << mysqlpp::quote << vals[i];
	}
}

void MysqlModule::Qinsert(mysqlpp::Query & q, const string & tab, SqlRow & fields, SqlRow & vals)
{
	q << "INSERT INTO " << tab << "(";
	for (int i = 0; i < fields.size(); ++i)
	{
		if (i == 0)
			q << fields[i];
		else
			q << ", " << fields[i];
	}
	q << ") VALUES(";
	for (int i = 0; i < vals.size(); ++i)
	{
		if (i == 0)
			q << mysqlpp::quote << vals[i];
		else
			q << ", " << mysqlpp::quote << vals[i];
	}
	q << ");";
}

void MysqlModule::Qselect(mysqlpp::Query & q, const string & tab, SqlRow & fields)
{
	q << "SELECT ";
	for (size_t i = 0; i < fields.size(); i++)
	{
		if (i > 0)
			q << ",";
		q << fields[i];
	}
	q << "FROM " << tab;
}

void MysqlModule::Qwhere(mysqlpp::Query & q, SqlRow & kname, SqlRow & kval)
{
	q << " WHERE ";
	for (size_t i = 0; i < kname.size(); i++)
	{
		if (i == 0)
			q  << kname[i] << " = " << mysqlpp::quote << kval[i];
		else
			q << " AND "  << kname[i] << " = " << mysqlpp::quote << kval[i];
	}
	q << ";";
}