#ifndef SEND_PROXY_DB_MODULE_H
#define SEND_PROXY_DB_MODULE_H

#include "BaseModule.h"
#include "Reflection.h"
#include "MysqlModule.h"
#include "MsgModule.h"

#include "protoPB/base/LPSql.pb.h"
#include "GameObject.h"

class TransMsgModule;

class SendProxyDbModule:public BaseModule
{
public:
	SendProxyDbModule(BaseLayer* l);
	~SendProxyDbModule();

	void SendToProxyDb(google::protobuf::Message& msg,const int32_t& hash,const int32_t& mid);
	void SendToProxyDbGroup(google::protobuf::Message& msg, const int32_t& groupId, const int32_t& mid);

	SHARE<BaseMsg> RequestToProxyDb(google::protobuf::Message& msg, const int32_t& hash, const int32_t& mid, c_pull& pull, SHARE<BaseCoro>& coro);
	SHARE<BaseMsg> RequestToProxyDbGroup(google::protobuf::Message& msg, const int32_t& groupId, const int32_t& mid, c_pull& pull, SHARE<BaseCoro>& coro);

	template<typename T>
	void MakeParamkey(Reflect<T>& rf,LPMsg::PBSqlParam& param)
	{
		auto p = (char*)rf.ptr;
		for (auto& k : TableDesc<T>::paramkey)
		{
			int32_t idx = Reflect<T>::GetFieldIndex(k);
			param.add_kname(k);
			param.add_kval(Reflect<T>::GetVal(p + Reflect<T>::arr_offset[idx], Reflect<T>::arr_type[idx]));
		}
	}

	template<typename T>
	void MakeFields(Reflect<T>& rf, LPMsg::PBSqlParam& param)
	{
		auto p = (char*)rf.ptr;
		for (size_t i = 0; i < Reflect<T>::Size(); i++)
		{
			if (rf.flag & (((int64_t)1) << i))
			{
				param.add_field(Reflect<T>::arr_fields[i]);
				param.add_value(Reflect<T>::GetVal(p + Reflect<T>::arr_offset[i], Reflect<T>::arr_type[i]));
			}
		}
	}

	template<typename T>
	void MakeFieldsKey(Reflect<T>& rf, LPMsg::PBSqlParam& param)
	{
		auto p = (char*)rf.ptr;
		for (size_t i = 0; i < Reflect<T>::Size(); i++)
		{
			if (rf.flag & (((int64_t)1) << i))
			{
				param.add_kname(Reflect<T>::arr_fields[i]);
				param.add_kval(Reflect<T>::GetVal(p + Reflect<T>::arr_offset[i], Reflect<T>::arr_type[i]));
			}
		}
	}

	template<typename T>
	void MakeUpdateSqlParam(Reflect<T>& rf, const int64_t& uid, LPMsg::PBSqlParam& param)
	{
		param.set_uid(uid);
		param.set_opt(SQL_OPT::SQL_UPDATE);
		param.set_table(Reflect<T>::Name());
		MakeParamkey(rf, param);
		MakeFields(rf, param);
	}

	template<typename T>
	void MakeSelectSqlParam(Reflect<T>& rf, const int64_t& uid, LPMsg::PBSqlParam& param,bool usefield=false)
	{
		param.set_uid(uid);
		param.set_opt(SQL_OPT::SQL_SELECT);
		param.set_table(Reflect<T>::Name());
		if (usefield)
			MakeFieldsKey(rf, param);
		else
			MakeParamkey(rf, param);
	}

	template<typename T>
	void MakeInsertSqlParam(Reflect<T>& rf, const int64_t& uid, LPMsg::PBSqlParam& param)
	{
		param.set_uid(uid);
		param.set_opt(SQL_OPT::SQL_INSERT);
		param.set_table(Reflect<T>::Name());
		MakeFields(rf, param);
	}

	template<typename T>
	void MakeInsertSelectSqlParam(Reflect<T>& rf, const int64_t& uid, LPMsg::PBSqlParam& param,bool usefield=false)
	{
		param.set_uid(uid);
		param.set_opt(SQL_OPT::SQL_INSERT_SELECT);
		param.set_table(Reflect<T>::Name());
		MakeFields(rf, param);
		if (usefield)
			MakeFieldsKey(rf, param);
	}

	template<typename T>
	SHARE<BaseMsg> RequestUpdateDbGroup(Reflect<T>& rf,const int64_t& uid,const int32_t& groupid,const int32_t& mid, c_pull& pull, SHARE<BaseCoro>& coro)
	{
		LPMsg::PBSqlParam param;
		MakeUpdateSqlParam(rf, uid, param);
		return RequestToProxyDbGroup(param, groupid, mid, pull, coro);
	}

	template<typename T>
	SHARE<BaseMsg> RequestUpdateDbGroup(Reflect<T>& rf, const int64_t& uid, const int32_t& mid, c_pull& pull, SHARE<BaseCoro>& coro)
	{
		return RequestUpdateDbGroup(rf, uid, uid >> 56, mid, pull, coro);
	}

	template<typename T>
	bool RequestSelectDbGroup(T& t,Reflect<T>& rf, const int64_t& uid, c_pull& pull, SHARE<BaseCoro>& coro,bool usefield=false)
	{
		LPMsg::PBSqlParam param;
		MakeSelectSqlParam(rf, uid, param,usefield);
		auto acksql = RequestToProxyDbGroup(param, uid >> 56, N_MYSQL_CORO_MSG, pull, coro);
		auto sqlmsg = (NetMsg*)acksql->m_data;
		PARSEPB_IF_FALSE(LPMsg::PBSqlParam, sqlmsg, m_msgModule)
		{
			LP_ERROR(m_msgModule) << "parse PBSqlParam Error";
			return false;
		}
		if (pbMsg.value_size() == 0)
			return false;
		SetObjectValue(t, pbMsg);
		return true;
	}

	template<typename T>
	SHARE<BaseMsg> RequestInsertDbGroup(Reflect<T>& rf, const int64_t& uid, c_pull& pull, SHARE<BaseCoro>& coro)
	{
		LPMsg::PBSqlParam param;
		MakeInsertSqlParam(rf, uid, param);
		return RequestToProxyDbGroup(param, uid >> 56, N_MYSQL_CORO_MSG, pull, coro);
	}

	template<typename T>
	bool RequestInsertSelectDbGroup(T& t,Reflect<T>& rf, const int64_t& uid, c_pull& pull, SHARE<BaseCoro>& coro,bool usefield=false)
	{
		LPMsg::PBSqlParam param;
		MakeInsertSelectSqlParam(rf, uid, param, usefield);
		auto acksql = RequestToProxyDbGroup(param, uid >> 56, N_MYSQL_CORO_MSG, pull, coro);
		auto sqlmsg = (NetMsg*)acksql->m_data;
		PARSEPB_IF_FALSE(LPMsg::PBSqlParam, sqlmsg, m_msgModule)
		{
			LP_ERROR(m_msgModule) << "parse PBSqlParam Error";
			return false;
		}
		if (pbMsg.value_size() == 0)
			return false;
		SetObjectValue(t, pbMsg);
		return true;
	}

	template<typename T>
	void UpdateDbGroup(Reflect<T>& rf, const int64_t& uid)
	{
		LPMsg::PBSqlParam param;
		MakeUpdateSqlParam(rf, uid, param);
		SendToProxyDbGroup(param, uid >> 56, N_MYSQL_MSG);
	}

	template<typename T>
	SHARE<T> CreateObject(vector<string>& field, vector<string>& val)
	{
		if (field.size() != val.size())
			return nullptr;

		SHARE<T> t = GetLayer()->GetSharedLoop<T>();
		for (size_t i = 0; i < field.size(); i++)
		{
			Reflect<T>::SetFieldValue(*t.get(),field[i],val[i]);
		}
		return t;
	}

	template<typename T>
	typename std::enable_if<!std::is_base_of<GameObject, T>::value>::type SetObjectValue(T& rf, LPMsg::PBSqlParam& param)
	{
		if (param.field_size() != param.value_size() || param.field_size() > Reflect<T>::Size())
			return;
		for (size_t i = 0; i < param.field_size(); i++)
		{
			Reflect<T>::SetFieldValue(rf, param.field(i), param.value(i));
		}
	}

	template<typename T>
	typename std::enable_if<std::is_base_of<GameObject, T>::value>::type SetObjectValue(T& rf, LPMsg::PBSqlParam& param)
	{
		if (param.field_size() != param.value_size() || param.field_size() > Reflect<T>::Size())
			return;
		for (size_t i = 0; i < param.field_size(); i++)
		{
			Reflect<T>::SetFieldValue(rf, param.field(i), param.value(i));
		}
		rf.CopySqlData();
	}

private:

	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;
	void SendToProxyDb(google::protobuf::Message& msg, const int32_t& hash, const int32_t& mid, const int32_t& api);
	SHARE<BaseMsg> RequestToProxyDb(google::protobuf::Message& msg, const int32_t& hash, const int32_t& mid, c_pull& pull, SHARE<BaseCoro>& coro, const int32_t& api);

private:
	TransMsgModule * m_tranModule;
	MsgModule* m_msgModule;

	ServerNode m_proxy;
};

#endif