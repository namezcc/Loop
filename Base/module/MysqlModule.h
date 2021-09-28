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
	SQL_INSERT_SELECT,
};

struct SqlParam:public BaseData,public LoopObject
{
	int opt;
	bool ret;
	string tab;
	SqlRow kname;
	SqlRow kval;
	SqlRow field;
	SqlRow value;

	virtual void initMsg()
	{};
	virtual void recycleMsg()
	{
		kname.clear();
		kval.clear();
		field.clear();
		value.clear();
	};
	virtual void init(FactorManager * fm) override
	{};
	virtual void recycle(FactorManager * fm) override { recycleMsg();};
};

struct LMsgSqlParam:public BaseData
{
	LMsgSqlParam():index(0),param(NULL)
	{}

	virtual void initMsg()
	{
	};
	virtual void recycleMsg()
	{
		if (param)
			param->recycleMsg();
	};

	uint32_t index;
	SqlParam* param;
};

class SqlResult
{
public:
	virtual int32_t getInt32(const char* f) = 0;
	virtual uint32_t getUInt32(const char* f) = 0;
	virtual int64_t getInt64(const char* f) = 0;
	virtual uint64_t getUInt64(const char* f) = 0;
	virtual std::string getString(const char* f) = 0;
	virtual float getFloat(const char* f) = 0;
	virtual double getDouble(const char* f) = 0;
	virtual bool eof() = 0;
	virtual void nextRow() = 0;
};

class MsgModule;

#if PLATFORM == PLATFORM_LINUX
#define sprintf_s snprintf
#endif // PLATF

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
		MultRow res;
		SqlRow fiels;
		char sql[256];
		std::string tabName = newName.empty() ? Reflect<T>::Name() : newName;
		sprintf_s(sql,sizeof(sql), "select COLUMN_NAME from information_schema.COLUMNS where table_name = '%s';", tabName.c_str());
		Select(sql, res, fiels);
		std::set<std::string> repeate;
		for (auto vec:res)
		{
			repeate.insert(vec.front());
		}
		TableQuery<T>::InitTable(newName,[this](string& sql) {
			Query(sql);
		}, repeate);
	}

	bool Query(const string& str);
	bool Query(const char* str);
	SHARE<SqlResult> query(const string& str);
	SHARE<SqlResult> query(const char* str);
	bool Select(const string& str,MultRow& res,SqlRow& files);

	bool Insert(SqlParam& p);
	bool Delete(SqlParam& p);
	bool Update(SqlParam& p);
	bool Select(SqlParam& p);

	template<typename T>
	bool Insert(T&t,const string& newName="")
	{
		//auto param = GetLayer()->GetSharedLoop<SqlParam>();
		auto param = GET_SHARE(SqlParam);
		if (!newName.empty())
			param->tab = newName;
		else
			param->tab = Reflect<T>::Name();
		param->field.assign(Reflect<T>::arr_fields.begin(), Reflect<T>::arr_fields.end());
		auto p = (char*)&t;
		for (size_t i = 0; i < Reflect<T>::Size(); i++)
		{
			param->value.push_back(move(Reflect<T>::GetVal(p+ Reflect<T>::arr_offset[i], Reflect<T>::arr_type[i])));
		}
		return Insert(*param);
	}

	template<typename T>
	bool Insert(Reflect<T>& rf, const string& newName = "")
	{
		//auto param = GetLayer()->GetSharedLoop<SqlParam>();
		auto param = GET_SHARE(SqlParam);
		if (!newName.empty())
			param->tab = newName;
		else
			param->tab = Reflect<T>::Name();

		auto p = (char*)rf.ptr;
		for (size_t i = 0; i < Reflect<T>::Size(); i++)
		{
			if (rf.flag & (((int64_t)1) << i))
			{
				param->field.push_back(Reflect<T>::arr_fields[i]);
				param->value.push_back(Reflect<T>::GetVal(p + Reflect<T>::arr_offset[i], Reflect<T>::arr_type[i]));
			}
		}
		return Insert(*param);
	}

	template<typename T>
	bool Update(Reflect<T>& rf, const string& newName = "")
	{
		//auto param = GetLayer()->GetSharedLoop<SqlParam>();
		auto param = GET_SHARE(SqlParam);
		if (!newName.empty())
			param->tab = newName;
		else
			param->tab = Reflect<T>::Name();
		param->kname.assign(TableDesc<T>::paramkey.begin(), TableDesc<T>::paramkey.end());
		auto p = (char*)rf.ptr;
		for (auto& k: TableDesc<T>::paramkey)
		{
			int idx = Reflect<T>::GetFieldIndex(k);
			param->kval.push_back(Reflect<T>::GetVal(p + Reflect<T>::arr_offset[idx], Reflect<T>::arr_type[idx]));
		}
		for (size_t i = 0; i < Reflect<T>::Size(); i++)
		{
			if (rf.flag & (((int64_t)1) << i))
			{
				param->field.push_back(Reflect<T>::arr_fields[i]);
				param->value.push_back(Reflect<T>::GetVal(p + Reflect<T>::arr_offset[i], Reflect<T>::arr_type[i]));
			}
		}
		return Update(*param);
	}

	template<typename T>
	bool Delete(T&t, const string& newName = "")
	{
		//auto param = GetLayer()->GetSharedLoop<SqlParam>();
		auto param = GET_SHARE(SqlParam);
		if (!newName.empty())
			param->tab = newName;
		else
			param->tab = Reflect<T>::Name();
		param->kname.assign(TableDesc<T>::paramkey.begin(), TableDesc<T>::paramkey.end());
		auto p = (char*)&t;
		for (auto& k : TableDesc<T>::paramkey)
		{
			int idx = Reflect<T>::GetFieldIndex(k);
			param->kval.push_back(Reflect<T>::GetVal(p + Reflect<T>::arr_offset[idx], Reflect<T>::arr_type[idx]));
		}
		return Delete(*param);
	}

	template<typename T>
	bool Select(T& t, const string& newName = "")
	{
		auto param = GET_SHARE(SqlParam);
		if (!newName.empty())
			param->tab = newName;
		else
			param->tab = Reflect<T>::Name();
		param->kname.assign(TableDesc<T>::paramkey.begin(), TableDesc<T>::paramkey.end());
		auto p = (char*)&t;
		for (auto& k : TableDesc<T>::paramkey)
		{
			int idx = Reflect<T>::GetFieldIndex(k);
			param->kval.push_back(Reflect<T>::GetVal(p + Reflect<T>::arr_offset[idx], Reflect<T>::arr_type[idx]));
		}
		if (Select(*param))
		{
			if (param->field.size() != param->value.size())
				return false;
			for (size_t i = 0; i < param->field.size(); i++)
				Reflect<T>::SetFieldValue(t, param->field[i], param->value[i]);
			return true;
		}
		return false;
	}

	template<typename T>
	bool Select(Reflect<T>& t, const string& newName = "")
	{
		return Select(*t.ptr,newName);
	}

	template<typename T>
	bool Select(vector<T>& res,const string& sql)
	{
		MultRow tmp;
		SqlRow files;
		if (!Select(sql, tmp, files))
			return false;
		for (size_t i = 0; i < tmp.size(); i++)
		{
			T t = {};
			for (size_t j = 0; j < files.size(); j++)
			{
				Reflect<T>::SetFieldValue(t, files[j], tmp[i][j]);
			}
			res.push_back(t);
		}
		return true;
	}

	inline int GetDBGroup() { return m_dbgroup; }
protected:
	// ͨ�� BaseModule �̳�
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
	int m_dbgroup;

	MsgModule* m_msgModule;

	SHARE<mysqlpp::Connection> m_sqlConn;
};

#endif