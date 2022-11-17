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

local conf = {
	{
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
    {
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
    {
        _table = "lp_player",
        _proto = "DB_player",
        _key = {2},
        _field = {
            {"uid",SQL_INT64},
            {"pid",SQL_INT64},
            {"name",SQL_STRING},
            {"level",SQL_INT},
            {"gold",SQL_INT},
        },
        _sql = {
            {_type = SQL_TYPE_INSERT,_vec=false,_update=true},
            {_type = SQL_TYPE_UPDATE,_vec=false},
            {_type = SQL_TYPE_SELECT,_vec=true,_key={1}},
            {_type = SQL_TYPE_SELECT,_vec=false},
        },
    },
	{
		_table = "lp_player_relation",
		_proto = "DB_player_relation",
		_field = {
			{"pid",SQL_INT64},
			{"rpid",SQL_INT64},
			{"name",SQL_STRING},
			{"level",SQL_INT},
			{"time",SQL_INT},
			{"type",SQL_INT},
		},
		_key = {1,2},
		_sql = {
			{_type = SQL_TYPE_SELECT,_vec=true,_key={1}},
			{_type = SQL_TYPE_INSERT,_vec=false,_update=true},
			{_type = SQL_TYPE_UPDATE,_vec=false},
			{_type = SQL_TYPE_DELETE,_vec=false},
		}
	},

}

return conf