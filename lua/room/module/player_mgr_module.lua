local _msg_func = MSG_FUNC

local player_mgr_module = {}
local ply_tab = ALL_PLAYER
local sock_tab = PLAYER_SOCK
local lfunc = LFUNC
local _log = LOG
local _pack = PACK
local _room_ser = ROOM_MGR_SERVER
local _SMSG_ID = SERVER_MSG

function player_mgr_module:init()
	
	self._net = MOD.net_mgr_module
	self._db = MOD.db_mgr_module

	self._player_tab = {}

	_msg_func.bind_mod_func(CTOL.CTOL_SET_PLAYER_SOCK,self,self.onSetPlayerSock)
	_msg_func.bind_mod_func(CTOL.CTOL_PLAYER_OFFLINE,self,self.onPlayerOffline)
	
	_msg_func.bind_mod_proto_func(SERVER_MSG.N_TROM_LOAD_ROLE_DATA,self,self.onLoadPlayerData,"DB_player_all_data")


end

function player_mgr_module:onLoadPlayerData(pdata)
	local uid = pdata.player.uid

	local sock = self:getPlayerSock(uid)
	if sock == nil then
		_log.error("player not have sock uid:",uid)
		return
	end

	_log.info("player load data uid:%d",uid)

	local ply = {}
	ply.uid = pdata.player.uid
	ply.pid = pdata.player.pid
	ply.sock = sock
	ply.player = table.clone(pdata.player)

	setPlayerImpFunc(ply)

	ply:initDB(pdata)

	ply_tab[sock] = ply
	self._player_tab[ply.pid] = ply

	self:sendPlayerOnline(ply.pid)
end

function player_mgr_module:onSetPlayerSock(uid,sock)
	if sock_tab[uid] ~= nil then
		_log.warn("old sock not nil uid:%d",uid)
	end

	_log.info("set player sock uid: %d,sock:%d",uid,sock)

	sock_tab[uid] = sock
end

function player_mgr_module:onPlayerOffline(uid,sock)
	sock_tab[uid] = nil

	local pack = _pack.buffpack()
	pack:writeint64(uid)

	local player = self:getPlayerBySock(sock)
	if player then

		player:logout()

		pack:writeint64(player.pid)
		ply_tab[sock] = nil
		self._player_tab[player.pid] = nil
		-- 保存数据库
		self._db:doPlayerDB_Opt(player.pid,SQL_OPRATION.SOP_SAVE_PlAYER_TO_DB)
	else
		pack:writeint64(0)
	end

	self._net:sendToServer(_room_ser,_SMSG_ID.N_TRMM_PLAYER_LOGOUT,pack)
end

function player_mgr_module:getPlayerSock(uid)
	return sock_tab[uid]
end

function player_mgr_module:getPlayerByUid(uid)
	local sock = sock_tab[uid]
	if sock then
		return ply_tab[sock]
	end
end

function player_mgr_module:getPlayerByPid(pid)
	return self._player_tab[pid]
end

function player_mgr_module:getPlayerBySock(sock)
	return ply_tab[sock]
end

function player_mgr_module:sendPlayerOnline(pid)
	local buff = _pack.buffpack()

	buff:writeint32(1)
	buff:writeint64(pid)

	self._net:sendToServer(_room_ser,_SMSG_ID.N_TRMM_PLAYER_ONLINE,buff)
end

return player_mgr_module