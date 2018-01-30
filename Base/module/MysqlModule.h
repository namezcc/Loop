#ifndef MYSQL_MODULE_H
#define MYSQL_MODULE_H
#include "BaseModule.h"

#define MYSQL_TRY try{

#define MYSQL_CATCH(msg) }\
	catch (mysqlpp::BadQuery er) \
    { \
        std::cout << "BadQuery [" << msg << "] Error: " << er.what() << std::endl; \
        return false; \
    } \
    catch (const mysqlpp::BadConversion& er)  \
    { \
        std::cout << "BadConversion [" << msg << "] Error:" << er.what() << " retrieved data size:" << er.retrieved << ", actual size:" << er.actual_size << std::endl; \
        return false; \
    } \
	catch (const mysqlpp::ConnectionFailed& er) \
	{ \
		std::cerr << "Failed to connect to database server: " <<er.what()  << std::endl; \
		return false; \
	} \
    catch (const mysqlpp::Exception& er) \
    { \
        std::cout << "mysqlpp::Exception [" << msg << "] Error:" << er.what() << std::endl; \
        return false; \
    }\
    catch ( ... ) \
    { \
        std::cout << "std::exception [" <<msg << "] Error:Unknown " << std::endl; \
        return false; \
    }

namespace mysqlpp {
	class Connection;
	class Query;
}

typedef std::vector<std::string> SqlRow;
typedef std::vector<SqlRow> MultRow;

enum SQL_OPT
{
	SQL_SELECT,
	SQL_INSERT,
	SQL_DELETE,
	SQL_UPDATE,
};

struct SqlParam:public LoopObject
{
	int opt;
	string tab;
	SqlRow kname;
	SqlRow kval;
	SqlRow field;
	SqlRow value;

	// 通过 LoopObject 继承
	virtual void init(FactorManager * fm) override;
	virtual void recycle(FactorManager * fm) override;
};

class LOOP_EXPORT MysqlModule:public BaseModule
{
public:
	MysqlModule(BaseLayer* l);
	~MysqlModule();

	bool Connect(const string& dbname,const string& ip, const string& user, const string& pass, const int& port=3306);
	bool Reconnect();

	bool Query(const string& str);
	bool Select(const string& str,MultRow& res,SqlRow& files);

	bool Inster(SqlParam& p);
	bool Delete(SqlParam& p);
	bool Update(SqlParam& p);
	bool Select(SqlParam& p);

protected:
	// 通过 BaseModule 继承
	virtual void Init() override;
	virtual void Execute() override;

	void Qupdate(mysqlpp::Query& q, const string& tab, SqlRow& fields, SqlRow& vals);
	void Qinsert(mysqlpp::Query& q, const string& tab, SqlRow& fields, SqlRow& vals);
	void Qselect(mysqlpp::Query& q, const string& tab, SqlRow& fields);
	void Qwhere(mysqlpp::Query& q, SqlRow& kname, SqlRow& kval);

private:

	string m_dbname;
	string m_ip;
	string m_user;
	string m_pass;
	int m_port;

	SHARE<mysqlpp::Connection> m_sqlConn;
};

#endif