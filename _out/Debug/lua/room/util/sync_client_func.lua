
local _sm = SM

SYNC_CLIENT_FUNC = {}

SYNC_CLIENT_FUNC[TABLE_INDEX.TAB_lp_player] = function (player,data)
	player:sendMsg(_sm.SM_PLAYER_INFO,data,"SmPlayerInfo")
end