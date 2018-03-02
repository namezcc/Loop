#ifndef MYSQL_MODULE_H
#define MYSQL_MODULE_H
#include "BaseModule.h"
#include "Reflection.h"

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

class MsgModule;

class LOOP_EXPORT MysqlModule:public BaseModule
{
public:
	MysqlModule(BaseLayer* l);
	~MysqlModule();

	void SetConnect(const string& dbname, const string& ip, const string& user, const string& pass, const int& port = 3306);
	bool Connect(const string& dbname,const string& ip, const string& user, const string& pass, const int& port=3306);
	bool Reconnect();

	template<typename T>
	void InitTable(string newName="")
	{
		TableQuery<T>::InitTable(newName,[this](string& sql) {
			Query(sql);
		});
	}

	bool Query(const string& str);
	bool Select(const string& str,MultRow& res,SqlRow& files);

	bool Inster(SqlParam& p);
	bool Delete(SqlParam& p);
	bool Update(SqlParam& p);
	bool Select(SqlParam& p);

	template<typename T>
	bool Insert(T&t)
	{
		auto param = GetLayer()->GetSharedLoop<SqlParam>();
		param->tab = Reflect<T>::Name();
		param->field.assign(Reflect<T>::arr_fields.begin(), Reflect<T>::arr_fields.end());
		auto p = (char*)&t;
		for (size_t i = 0; i < Reflect<T>::Size(); i++)
		{
			param->value.push_back(move(Reflect<T>::GetVal(p+ Reflect<T>::arr_offset[i], Reflect<T>::arr_type[i])));
		}
		return Inster(*param.get());
	}

	template<typename T>
	bool Update(Reflect<T>& rf)
	{
		auto param = GetLayer()->GetSharedLoop<SqlParam>();
		param->tab = Reflect<T>::Name();
		param->kname.assign(TableDesc<T>::paramkey.begin(), TableDesc<T>::paramkey.end());
		auto p = (char*)rf.mptr;
		for (auto& k: TableDesc<T>::paramkey)
		{
			int idx = Reflect<T>::GetFieldIndex(k);
			param->kval.push_back(Reflect<T>::GetVal(p + Reflect<T>::arr_offset[i], Reflect<T>::arr_type[i]));
		}
		for (size_t i = 0; i < Reflect<T>::Size(); i++)
		{
			if (rf.changeFlag & (((int64_t)1) << i))
			{
				param->field.push_back(Reflect<T>::arr_fields[i]);
				param->value.push_back(Reflect<T>::GetVal(p + Reflect<T>::arr_offset[i], Reflect<T>::arr_type[i]))
			}
		}
		return Update(*param.get());
	}

	template<typename T>
	bool Delete(T&t)
	{
		auto param = GetLayer()->GetSharedLoop<SqlParam>();
		param->tab = Reflect<T>::Name();
		param->kname.assign(TableDesc<T>::paramkey.begin(), TableDesc<T>::paramkey.end());
		auto p = (char*)&t;
		for (auto& k : TableDesc<T>::paramkey)
		{
			int idx = Reflect<T>::GetFieldIndex(k);
			param->kval.push_back(Reflect<T>::GetVal(p + Reflect<T>::arr_offset[i], Reflect<T>::arr_type[i]));
		}
		return Delete(*param.get());
	}

	template<typename T>
	bool Select(vector<SHARE<T>>& res,const string& sql)
	{
		MultRow tmp;
		SqlRow files;
		if (!Select(sql, tmp, files))
			return false;
		for (size_t i = 0; i < tmp.size(); i++)
		{
			SHARE<T> t = GetLayer()->GetSharedLoop<T>();
			for (size_t j = 0; j < files.size(); j++)
			{
				Reflect<T>::SetFieldValue(*t.get(), files[j], tmp[i][j]);
			}
			res.push_back(t);
		}
		return true;
	}

protected:
	// 通过 BaseModule 继承
	virtual void Init() override;
	virtual void AfterInit()override;
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

	MsgModule* m_msgModule;

	SHARE<mysqlpp::Connection> m_sqlConn;
};

#endif