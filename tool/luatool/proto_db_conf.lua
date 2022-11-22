SQL_INT = 1
SQL_INT64 = 2
SQL_STRING = 3
SQL_FLOAT = 4
SQL_TIMESTAMP = 5

SQL_TYPE_INSERT = 1
SQL_TYPE_DELETE = 2
SQL_TYPE_UPDATE = 3
SQL_TYPE_SELECT = 4
SQL_TYPE_REPLACE = 5


SQL_GT = 1		-- >
SQL_GTE = 2		-- >=
SQL_LT = 3		-- <
SQL_LTE = 4		-- <=
SQL_NE = 5		-- <>

--[[
[表id] =
_table:表名
_proto:协议名
_key:表的主键 _field 的index值 # = [1]
_rediskey:redis的键 除了 cid 之外的 {协议字段名...}
_field:{
	[1] = {表字段,数据类型,默认值(默认不填)}
	...
}
_sql:{
	_type:sql操作,
	_vec:是否多条数据,
	_key:sql的主键同上面的key,
	_update: insert 时是否加 ON DUPLICATE KEY UPDATE
	_field:{...} 同上 只更新指定字段
	_logic:{key=SQL_GT..} 指定where 时的逻辑运算符,默认都是 =
	_name:函数名后缀
},
...
]]

local conf = {
	[1] = {
        _table = "db_player_num_info",
        _proto = "DB_player_num_info",
        _key = {1},
        _field = {
            {"dbid",SQL_INT},
            {"num",SQL_INT},
            {"maxnum",SQL_INT},
			{"hostid",SQL_INT},
        },
        _sql = {
            {_type = SQL_TYPE_UPDATE},
            {_type = SQL_TYPE_SELECT,_vec=true,_key={}},
        },
    },
    [2] = {
        _table = "account",
        _proto = "DB_account",
        _key = {1},
        _field = {
            {"platform_uid",SQL_STRING},
            {"game_uid",SQL_INT64},
            {"create_time",SQL_INT},
        },
        _sql = {
            {_type = SQL_TYPE_INSERT},
            {_type = SQL_TYPE_SELECT,_vec=true,_key={}},
        },
    },
    [3] = {
        _table = "lp_player",
        _proto = "DB_player",
        _key = {2},
        _field = {
            {"cid",SQL_INT},
            {"uid",SQL_INT},
            {"name",SQL_STRING},
            {"level",SQL_INT},
            {"gold",SQL_INT},
			{"login_time",SQL_INT},
			{"logout_time",SQL_INT},
			{"create_time",SQL_INT},
        },
        _sql = {
            {_type = SQL_TYPE_INSERT},
            {_type = SQL_TYPE_UPDATE},
            {_type = SQL_TYPE_SELECT,_vec=true,_key={2}},
            {_type = SQL_TYPE_SELECT},
        },
    }

}

-- log的表
LOG_CONF = {

}

return conf