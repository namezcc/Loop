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
        _table = "account",
        _proto = "DB_account",
        _key = {1},
        _field = {
            [1] = {"platform_uid","platform_uid",SQL_STRING},
            [2] = {"hash_index","hash_index",SQL_INT},
            [3] = {"game_uid","game_uid",SQL_INT64},
            [4] = {"dbindex","dbindex",SQL_INT},
        },
        _sql = {
            {_type = SQL_TYPE_INSERT,_vec=false},
            -- {_type = SQL_TYPE_REPLACE,_vec=false},
            -- {_type = SQL_TYPE_DELETE,_vec=false},
            {_type = SQL_TYPE_UPDATE,_vec=false},
            {_type = SQL_TYPE_SELECT},
        },
    },
    {
        _table = "lp_player",
        _proto = "DB_player",
        _key = {2},
        _field = {
            [1] = {"uid","uid",SQL_INT64},
            [2] = {"pid","pid",SQL_INT64},
            [3] = {"name","name",SQL_STRING},
            [4] = {"level","level",SQL_INT},
            [5] = {"gold","gold",SQL_INT},
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
			[1] = {"pid","pid",SQL_INT64},
			[2] = {"rpid","rpid",SQL_INT64},
			[3] = {"name","name",SQL_STRING},
			[4] = {"level","level",SQL_INT},
			[5] = {"time","time",SQL_INT},
			[6] = {"type","type",SQL_INT},
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