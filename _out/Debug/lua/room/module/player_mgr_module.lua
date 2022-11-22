local _msg_func = MSG_FUNC

local player_mgr_module = {}
local _player_table = ALL_PLAYER
local _player_gate = PLAYER_GATE
local lfunc = LFUNC
local _log = LOG
local _pack = PACK
local _room_ser = ROOM_MGR_SERVER
local _SMSG_ID = SERVER_MSG
local _sm = SM
local _cm = CM

local _pb = pb

local _net = nil
local _db = nil

function player_mgr_module:init()
	_net = MOD.net_mgr_module
	_db = MOD.db_mgr_module
	self._player = {}


end

function player_mgr_module:init_func()
	_msg_func.bind_mod_func(CTOL.CTOL_SET_PLAYER_GATE_INFO,self,self.onSetPlayerGate)
	_msg_func.bind_mod_func(CTOL.CTOL_PLAYER_OFFLINE,self,self.onPlayerOffline)
	_msg_func.bind_mod_proto_func(SERVER_MSG.IM_ROOM_LOAD_ROLE_INFO,self,self.onLoadPlayerData,"DB_player_all_data")
end

function player_mgr_module:onSetPlayerGate(uid,gateid)
	_player_gate[uid] = gateid
end

function player_mgr_module:onLoadPlayerData(pdata)
	local uid = pdata.player.uid
	local cid = pdata.player.cid
	local gateid = _player_gate[uid]
	_log.info("player load data uid:%d cid:%d gateid:%d",uid,cid,gateid)

	local player = {}
	player.uid = uid
	player.cid = cid
	player.gateid = gateid

	setPlayerImpFunc(player)

	player:initDB(pdata)
	player:afterInit()

	local sendmsg = {}
	player:sendClienMsg(sendmsg)
	player:sendMsg(_sm.SM_PLAYER_ALL_INFO,sendmsg,"SmPlayerAllMsg")

	_player_table[uid] = player
	self._player[cid] = player
end

function player_mgr_module:onPlayerOffline(uid)
	local player = self:getPlayerByUid(uid)
	if player == nil then
		return
	end
	player:logout()
	_player_table[uid] = nil
	self._player[player.cid] = nil
	_log.info("player offline uid:%d cid:%d",uid,player.cid)
	-- 保存数据库
	_db:doPlayerDB_Opt(player.cid,SQL_OPRATION.SOP_SAVE_PlAYER_TO_DB)
end

function player_mgr_module:getPlayerByCid(cid)
	return self._player[cid]
end

function player_mgr_module:getPlayerByUid(uid)
	return _player_table[uid]
end

return player_mgr_module