
local _proto_name = TABLE_PROTO_NAME
local _rediskey = REDIS_KEY
local _log = LOG
local _pbencode = pb.encode

SYNC_FUNC = {}
local function set_data(tab,data)
	for k, v in pairs(data) do
		tab[k] = v
	end
end

local function get_sql_data(table,opt,pb)
	local pro = _proto_name[table]
	if pro == nil then
		_log.error("update player err proname nil table:%d",table)
		_log.info(debug.traceback())
		return
	end

	local redisk = _rediskey[table]
	if redisk == nil then
		_log.error("redis key nil table:%d",table)
		return
	end

	local ks = {}
	local buf = _pbencode(pro,pb)
	if buf == nil then
		return
	end

	for ik, k in ipairs(redisk) do
		ks[ik] = tostring(pb[k])
	end
	local data = {
		table = table,
		opt = opt,
		keys = ks,
		data = buf,
	}

	return data
end

SYNC_FUNC[TABLE_INDEX.TAB_lp_player] = function (player,table,opt,data,dbvec)
	set_data(player._player_info,data)
	dbvec[#dbvec+1] = get_sql_data(table,opt,player._player_info)
	player:clientUpdateData(table,player._player_info)
end

