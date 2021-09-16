#include "MysqlModule.h"
#include <mysql++/mysql++.h>
#include "MsgModule.h"
#include "LoopServer.h"

#define MYSQL_TRY try{


#define _MYSQL_CATCH(msg,code) }\
	catch (mysqlpp::BadQuery& er) \
    { \
        LP_ERROR<<"BadQuery ["<<msg<<"] Error: "<<er.what(); \
        code \
    } \
    catch (const mysqlpp::BadConversion& er)  \
    { \
		LP_ERROR<<" retrieved data size:"<<er.retrieved<<", actual size:"<<er.actual_size;	\
        code \
    } \
	catch (const mysqlpp::ConnectionFailed& er) \
	{ \
		LP_ERROR<<"Failed to connect to database server: "<<er.what();	\
		code \
	} \
    catch (const mysqlpp::Exception& er) \
    { \
		LP_ERROR<<"mysqlpp::Exception ["<<msg<<"] Error:"<<er.what();	\
        code \
    }\
    catch ( ... ) \
    { \
		LP_ERROR<<"std::exception ["<<msg<<"] Error:Unknown ";	\
        code \
    }


#define MYSQL_CATCH(msg) _MYSQL_CATCH(msg,return false;)

#define MYSQL_CATCH_NOR() _MYSQL_CATCH("", )

class QuerySqlResult:public SqlResult
{
public:
	QuerySqlResult():m_index(0)
	{
	}

	~QuerySqlResult()
	{
	}


	// 通过 SqlResult 继承
	virtual int32_t getInt32(const char * f) override
	{
		auto i = m_ret.field_num(f);
		if (i < 0 || i >= m_ret.num_fields())
			return 0;
		return loop_cast<int32_t>(m_ret[m_index][i]);
	}

	virtual uint32_t getUInt32(const char * f) override
	{
		auto i = m_ret.field_num(f);
		if (i < 0 || i >= m_ret.num_fields())
			return 0;
		return loop_cast<uint32_t>(m_ret[m_index][i]);
	}

	virtual int64_t getInt64(const char * f) override
	{
		auto i = m_ret.field_num(f);
		if (i < 0 || i >= m_ret.num_fields())
			return 0;
		return loop_cast<int64_t>(m_ret[m_index][i]);
	}

	virtual uint64_t getUInt64(const char * f) override
	{
		auto i = m_ret.field_num(f);
		if (i < 0 || i >= m_ret.num_fields())
			return 0;
		return loop_cast<uint64_t>(m_ret[m_index][i]);
	}

	virtual std::string getString(const char * f) override
	{
		auto i = m_ret.field_num(f);
		if (i < 0 || i >= m_ret.num_fields())
			return 0;
		return std::string(m_ret[m_index][i]);
	}

	virtual float getFloat(const char* f) override
	{
		auto i = m_ret.field_num(f);
		if (i < 0 || i >= m_ret.num_fields())
			return 0;
		return loop_cast<float>(m_ret[m_index][i]);
	}

	virtual double getDouble(const char* f) override
	{
		auto i = m_ret.field_num(f);
		if (i < 0 || i >= m_ret.num_fields())
			return 0;
		return loop_cast<double>(m_ret[m_index][i]);
	}

	virtual bool eof() override
	{
		return m_index >= m_ret.num_rows();
	}

	virtual void nextRow() override
	{
		++m_index;
	}

	mysqlpp::StoreQueryResult m_ret;
	int32_t m_index;
};

MysqlModule::MysqlModule(BaseLayer * l):BaseModule(l)
{
	m_sqlConn.reset(new mysqlpp::Connection(true));
}

MysqlModule::~MysqlModule()
{
}

void MysqlModule::Init()
{
	m_msgModule = GetLayer()->GetModule<MsgModule>();

	auto& config = GetLayer()->GetLoopServer()->GetConfig();
	auto& sql = config.sql;
	m_dbgroup = sql.dbGroup;
	SetConnect(sql.db,sql.ip,sql.user,sql.pass,sql.port);

	if (!m_sqlConn->connected())
		Reconnect();
}

void MysqlModule::AfterInit()
{
	
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

	bool res = false;
	try
	{
		res = m_sqlConn->connect(m_dbname.data(), m_ip.data(), m_user.data(), m_pass.data(), m_port);
	}
	catch(const std::exception& e)
	{
		LP_ERROR << "Mysql Connect Error "<<e.what();
	}
	if (!res)
		LP_ERROR << "Mysql Connect Error "<<"db:"<<m_dbname<<" ip:"<<m_ip<<" user:"<<m_user<<" pass:"<<m_pass<<" port:"<<m_port;
	else
		LP_INFO << "Mysql Connect Success " << "db:" << m_dbname << " ip:" << m_ip << " port:" << m_port;
	return res;
}

bool MysqlModule::Query(const string & str)
{
	//bool ret = false;
	//auto query = m_sqlConn->query(str);

	//ExitCall _call([this,&ret,&str]() {
	//	if (!ret)
	//		LP_ERROR<< "Query Error:"<< str;
	//	//cout << "Query Error:" << str <<endl;
	//});
	//MYSQL_TRY
	//	auto r = query.execute();
	//MYSQL_CATCH("")
	//return ret = true;
	return Query(str.data());
}

bool MysqlModule::Query(const char * str)
{
	bool ret = false;
	auto query = m_sqlConn->query(str);

	ExitCall _call([this, &ret, &str]() {
		if (!ret)
			LP_ERROR << "Query Error:" << str;
		//cout << "Query Error:" << str <<endl;
	});
	MYSQL_TRY
		auto r = query.execute();
	MYSQL_CATCH("")
	return ret = true;
}

SHARE<SqlResult> MysqlModule::query(const string & str)
{
	/*bool ret = false;
	auto query = m_sqlConn->query(str);

	ExitCall _call([this, &ret, &str]() {
		if (!ret)
			LP_ERROR << "Query Error:" << str;
	});

	auto qres = new QuerySqlResult();
	SHARE<SqlResult> res = SHARE<SqlResult>(qres, [](SqlResult* _r) {
		delete (QuerySqlResult*)_r;
	});

	MYSQL_TRY
		qres->m_ret = query.store();
	MYSQL_CATCH_NOR()
	ret = true;
	return res;*/
	return query(str.data());
}

SHARE<SqlResult> MysqlModule::query(const char * str)
{
	bool ret = false;
	auto query = m_sqlConn->query(str);

	ExitCall _call([this, &ret, &str]() {
		if (!ret)
			LP_ERROR << "Query Error:" << str;
	});

	auto qres = new QuerySqlResult();
	SHARE<SqlResult> res = SHARE<SqlResult>(qres, [](SqlResult* _r) {
		delete (QuerySqlResult*)_r;
	});

	MYSQL_TRY
		qres->m_ret = query.store();
	MYSQL_CATCH_NOR()
	ret = true;
	return res;
}

bool MysqlModule::Select(const string & str, MultRow & res, SqlRow & files)
{
	bool ret = false;
	auto query = m_sqlConn->query(str);

	ExitCall _call([this,&ret, &str]() {
		if (!ret)
			LP_ERROR<< "Select Error:"<< str;
		//cout << "Select Error:" << str << endl;
	});
	MYSQL_TRY
		auto result = query.store();
		if (result)
		{
			for (int i = 0; i < result.num_fields(); i++)
				files.push_back(result.field_name(i));

			for (int i = 0; i < result.num_rows(); i++)
			{
				SqlRow row;
				for (int j = 0; j < result.num_fields(); j++)
				{
					row.push_back(result[i][j].data());
				}
				res.push_back(std::move(row));
			}
		}
	MYSQL_CATCH("")
	return ret = true;
}

bool MysqlModule::Insert(SqlParam & p)
{
	bool ret = false;
	auto q = m_sqlConn->query();
	Qinsert(q, p.tab, p.field, p.value);
	ExitCall _call([this,&ret,&q]() {
		if (!ret)
			LP_ERROR<<"Inster Error:"<<q.str();
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
			LP_ERROR<< "Delete Error:"<< q.str();
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
			LP_ERROR<< "Update Error:"<<q.str();
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
			LP_ERROR<<"Select Error:"<< q.str();
		//cout << "Select Error:" << q.str() << endl;
	});
	MYSQL_TRY
		auto res = q.store();
		if (res)
		{
			if(p.field.size()==0)
				for (int i = 0; i < res.num_fields(); i++)
					p.field.push_back(res.field_name(i));
			for (int i = 0; i < res.num_rows(); i++)
				for (int j = 0; j < res.num_fields(); j++)
					p.value.push_back(res[i][j].data());
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
	for (size_t i = 0; i < fields.size(); ++i)
	{
		if (i == 0)
			q << fields[i];
		else
			q << ", " << fields[i];
	}
	q << ") VALUES(";

	size_t row = vals.size() / fields.size();

	for (size_t r = 0; r < row; r++)
	{
		if (r > 0)
			q << ",(";
		for (int i = 0; i < vals.size(); ++i)
		{
			if (i == 0)
				q << mysqlpp::quote << vals[i];
			else
				q << ", " << mysqlpp::quote << vals[i];
		}
		q << ")";
	}
	q << ";";
}

void MysqlModule::Qselect(mysqlpp::Query & q, const string & tab, SqlRow & fields)
{
	q << "SELECT ";
	if (fields.size() == 0)
		q << "*";
	else
	{
		for (size_t i = 0; i < fields.size(); i++)
		{
			if (i > 0)
				q << ",";
			q << fields[i];
		}
	}
	q << " FROM " << tab;
}

void MysqlModule::Qwhere(mysqlpp::Query & q, SqlRow & kname, SqlRow & kval)
{
	if (kname.size() > 0)
	{
		q << " WHERE ";
		for (size_t i = 0; i < kname.size(); i++)
		{
			if (i == 0)
				q << kname[i] << " = " << mysqlpp::quote << kval[i];
			else
				q << " AND " << kname[i] << " = " << mysqlpp::quote << kval[i];
		}
	}
	q << ";";
}