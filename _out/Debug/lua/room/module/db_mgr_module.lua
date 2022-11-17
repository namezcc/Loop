local db_mgr_module = {}
local _func = LFUNC
local _lc = LTOC
local _smsg = SERVER_MSG
local _msg_f = MSG_FUNC
local _log = LOG
local _pack = PACK

function db_mgr_module:init()

	self._ply_mgr = MOD.player_mgr_module
	self._db_ack = {}

	_msg_f.bind_mod_pack_func(_smsg.N_TROM_DB_PLAYER_OPERATION,self,self.onDbPlayerOperation)
end

function db_mgr_module:bind_player_db_ack_func(opt,f)
	self._db_ack[opt] = function (pid,pack)
		local player = self._ply_mgr:getPlayerByPid(pid)
		if player then
			f(player,pack)
		else
			_log.error("opt ack player nil pid:%d",pid)
		end
	end
end

function db_mgr_module:bind_player_db_ack_proto_func(opt,f,proto)
	local _pro = "LPMsg."..proto
	self._db_ack[opt] = function (pid,pack)
		local player = self._ply_mgr:getPlayerByPid(pid)
		if player then
			local pdata = pack:decode(_pro)
			f(player,pdata)
		else
			_log.error("opt ack player nil pid:%d",pid)
		end
	end
end

function db_mgr_module:doPlayerDB_Opt(pid,opt,pack)
	local buff = nil
	if pack then
		buff = pack:buff()
	end
	_func.CallCFunc(_lc.LTOC_DO_SQL_OPERATION,pid,opt,buff,_smsg.N_TROM_DB_PLAYER_OPERATION)
end

function db_mgr_module:dbUpdatePlayerData(pid,tabindex,pb,proto,k1,k2)
	k1 = k1 or "0"
	k2 = k2 or "0"

	-- if type(k1) ~= "string" then
	-- 	k1 = tostring(k1)
	-- end

	-- if type(k2) ~= "string" then
	-- 	k2 = tostring(k2)
	-- end

	proto = "LPMsg."..proto
	local pbstr = _pack.encodePB(pb,proto)

	_func.CallCFunc(_lc.LTOC_SQL_UPDATE_PLAYER_DATA,pid,tabindex,k1,k2,pbstr)
end

function db_mgr_module:dbDeletePlayerData(pid,tabindex,pb,proto,k1,k2)
	k1 = k1 or "0"
	k2 = k2 or "0"
	proto = "LPMsg."..proto
	local pbstr = _pack.encodePB(pb,proto)

	_func.CallCFunc(_lc.LTOC_SQL_DELETE_PLAYER_DATA,pid,tabindex,k1,k2,pbstr)
end

function db_mgr_module:onDbPlayerOperation(netpack)

	local pid = netpack:readint64()
	local opt = netpack:readint32()
	local ack = netpack:readint32()

	local f = self._db_ack[opt]
	if f then
		f(pid,netpack)
	else
		_log.error("dbplayer operation opt %d ack nil",opt)
	end
end

return db_mgr_module