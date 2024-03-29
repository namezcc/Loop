--文件是自动生成, tool/make_msg_enum.bat
CM = {
	CM_MSG_NONE = 0,
	CM_BEGAN = 10000,
	CM_LOGIN = 10001,
    CM_ENTER_ROOM = 10002,
    CM_CREATE_ROLE = 10003,
    CM_BEGIN_MATCH   = 10004,
    CM_STOP_MATCH    = 10005,
    CM_ENTER_BATTLE_SCENE    = 10006,
    CM_PLAYER_OPERATION      = 10007,
    CM_FIX_FRAME             = 10008,
	CM_ENTER_GAME = 10009,
	CM_SEARCH_PLAYER = 10010,
	CM_ADD_FRIEND = 10011,
	CM_OPT_FRIEND_APPLY = 10012,
	CM_DELETE_FRIEND = 10013,
	CM_SYNC_RELATION_INFO = 10014,			--同步信息
	CM_TEAM_CREATE = 10015,
	CM_TEAM_ASK_JOIN = 10016,				
	CM_TEAM_REPLY_JOIN = 10017,				
	CM_TEAM_QUIT = 10018,					
	CM_TEAM_DISBAND = 10019,				
	CM_TEAM_KICK = 10020,					--pid
	CM_TEAM_INVAITE = 10021,				--pid
	CM_TEAM_REPLY_INVAITE = 10022,			--pid
	
	CM_DEBUG = 10023,

	CM_END = 15000,
}
SM = {
	SM_MSG_NONE = 0,
	SM_BEGIN = 15000,
	SM_LOGIN_RES = 15001,
	SM_ENTER_ROOM = 15002,
    SM_CREATE_ROLE = 15003,
    SM_SELF_ROLE_INFO		= 15004,
    SM_PLAYER_ALL_INFO		= 15005,
    SM_PLAYER_INFO 		= 15006,
    
    



	SM_END = 20000,
}
SERVER_MSG = {
	IM_MSG_NONE = 0,
	IM_MSG_BEGIN = 1000,
	
	IM_ACCOUNT_GET_UID = 1001,
	
	IM_RMGR_PLAYER_LOGIN = 1101,
	IM_RMGR_PLAYER_ONLINE_NUM = 1102,
	IM_RMGR_PLAYER_LOGOUT = 1103,
	
	IM_GATE_PLAYER_LOGIN = 1201,
	
	IM_ROOM_PLAYER_LOGIN = 1301,
	IM_ROOM_PLAYER_LOGOUT = 1302,
	IM_ROOM_GET_ROLE_LIST = 1303,
	IM_ROOM_LOAD_ROLE_INFO = 1304,
	IM_ROOM_REG_DBID = 1305,

	IM_DB_SQL_OPERATION = 1401,
	
	IM_MSG_END = 5000,
}
