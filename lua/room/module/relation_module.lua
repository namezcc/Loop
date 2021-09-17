local relation_module = {}
local _sm = SM
local _cm = CM
local _msg_func = MSG_FUNC
local _db_mgr = nil
local _net_mgr = nil
local _pack = PACK
local _smsg = SERVER_MSG
local _room_mgr = ROOM_MGR_SERVER
local _log = LOG

local RL_TYPE = {
	RL_NONE = -1,
	RL_ASK_FRIEND = 0,
	RL_FRIEND = 1,
}

function relation_module:init()
	
	_db_mgr = MOD.db_mgr_module
	_net_mgr = MOD.net_mgr_module

	_msg_func.bind_player_proto_func(_cm.CM_SEARCH_PLAYER,self.onSearchPlayer,"propertyInt64")
	_msg_func.bind_player_proto_func(_cm.CM_ADD_FRIEND,self.onAskAddFriend,"propertyInt64")
	_msg_func.bind_player_proto_func(_cm.CM_OPT_FRIEND_APPLY,self.onReplyFriend,"ReplyFriend")
	_msg_func.bind_player_proto_func(_cm.CM_DELETE_FRIEND,self.onDeleteFriend,"propertyInt64")

	_msg_func.bind_player_server_proto(_smsg.N_TROM_ASK_ADD_FRIEND,self.onGetAskAddFriend,"DB_player_relation")
	_msg_func.bind_player_server_proto(_smsg.N_TROM_AGREE_ADD_FRIEND,self.onAgreeAddFriend,"DB_player_relation")
	

	_msg_func.bind_player_server_msg(_smsg.N_TRMM_CHECK_PLAYER_ONLINE,self.onCheckPlayerOnline)

	_db_mgr:bind_player_db_ack_proto_func(SQL_OPRATION.SOP_SEARCH_PLAYER,self.onGetSearchPlayer,"DB_player")
	

end

function relation_module:initDB(pdata)

	-- print("relation_module init uid:%d",self.uid)

	self._realtion = {}

	if #pdata.relation == 0 then
		self:sendRelation()
	else
		local pack = _pack.buffpack()
		pack:writeint64(self.pid)
		pack:writeint32(1)
		pack:writeint32(#pdata.relation)

		for i, v in ipairs(pdata.relation) do
			self._realtion[v.rpid] = table.clone(v)
			pack:writeint64(v.rpid)
		end

		self:sendToServer(_room_mgr,_smsg.N_TRMM_CHECK_PLAYER_ONLINE,pack)
	end
end

function relation_module:sendRelation(pid)
	local msg = {}

	if pid then
		local d = self._realtion[pid]
		if d == nil then
			return
		end

		local info = {
			pid = d.rpid,
			name = d.name,
			level = d.level,
			online = d.online,
			type = d.type,
		}

		msg[1] = info
	else

		for index, value in pairs(self._realtion) do
			local info = {
				pid = value.rpid,
				name = value.name,
				level = value.level,
				online = value.online,
				type = value.type,
			}
			msg[#msg+1] = info
		end
	end

	msg = {
		data = msg,
	}
	self:sendMsg(_sm.SM_RELATION_INFO,msg,"PlayerSimpleInfoList")
end

function relation_module:getRelation(pid)
	return self._realtion[pid]
end

function relation_module:isFriend(pid)
	local r = self._realtion[pid]
	if r and r.type == RL_TYPE.RL_FRIEND then
		return true
	else
		return false
	end
end

function relation_module:onSearchPlayer(pdata)
	
	local pack = _pack.buffpack()
	pack:writeint64(pdata.data)

	print("onSearchPlayer %d",pdata.data)
	_db_mgr:doPlayerDB_Opt(self.pid,SQL_OPRATION.SOP_SEARCH_PLAYER,pack)
end

function relation_module:onGetSearchPlayer(pdata)
	
	print("onGetSearchPlayer")
	printTable(pdata)

	self._online_cash = {pdata}

	local pack = _pack.buffpack()

	pack:writeint64(self.pid)
	pack:writeint32(0)
	pack:writeint32(1)
	pack:writeint64(pdata.pid)

	self:sendToServer(_room_mgr,_smsg.N_TRMM_CHECK_PLAYER_ONLINE,pack)
end

function relation_module:onCheckPlayerOnline(pack)
	local type = pack:readint32() 
	local num = pack:readint32()
	local online = {}

	for i = 1, num do
		local pid = pack:readint64()
		online[pid] = pack:readint8()
	end

	if type == 0 then
		if self._online_cash == nil then
			return
		end
		
		local pb = {}
		for i, v in ipairs(self._online_cash) do
			local info = {
				pid = v.pid,
				name = v.name,
				level = v.level,
				online = online[v.pid]
			}
			pb[#pb+1] = info
		end
	
		self._online_cash = nil
	
		self:sendMsg(_sm.SM_RELATION_INFO,{data = pb},"PlayerSimpleInfoList")
	else
		for k, v in pairs(online) do
			self._realtion[k].online = online[k]
		end
		self:sendRelation()
	end
end

function relation_module:onAskAddFriend(pdata)
	if pdata.data == self.pid then
		return
	end

	if self:isFriend(pdata.data) then
		return
	end

	local fpb = {
		pid = pdata.data,
		rpid = self.pid,
		type = RL_TYPE.RL_ASK_FRIEND,
		name = self.player.name,
		level = self.player.level,
	}
	_db_mgr:dbUpdatePlayerData(pdata.data,TABLE_INDEX.TAB_lp_player_relation,fpb,"DB_player_relation",fpb.rpid)

	-- 通知在线玩家
	_net_mgr:sendPbMsgToPlayer(pdata.data,_smsg.N_TROM_ASK_ADD_FRIEND,fpb,"DB_player_relation")	
end

function relation_module:onGetAskAddFriend(pdata)
	print("onGetAskAddFriend")
	printTable(pdata)

	if self:isFriend(pdata.rpid) then
		_log.info("allready friend pid:%d",pdata.rpid)
		return
	end

	self._realtion[pdata.rpid] = pdata
	self:sendRelation(pdata.rpid)
end

function relation_module:onAgreeAddFriend(pdata)
	print("onAgreeAddFriend")
	printTable(pdata)

	self._realtion[pdata.rpid] = pdata
	self:sendRelation(pdata.rpid)
end

function relation_module:onReplyFriend(pdata)
	local r = self:getRelation(pdata.pid)

	if r == nil then
		_log.error("not relation p:%d",pdata.pid)
		return
	end

	if r.type ~= RL_TYPE.RL_ASK_FRIEND then
		return
	end

	if pdata.res == 1 then
		r.type = RL_TYPE.RL_FRIEND
		_db_mgr:dbUpdatePlayerData(self.pid,TABLE_INDEX.TAB_lp_player_relation,r,"DB_player_relation",r.rpid)
		self:sendRelation(pdata.pid)

		-- 向对方插入数据并通知
		local fpb = {
			pid = pdata.pid,
			rpid = self.pid,
			type = RL_TYPE.RL_FRIEND,
			name = self.player.name,
			level = self.player.level,
		}
		_db_mgr:dbUpdatePlayerData(pdata.pid,TABLE_INDEX.TAB_lp_player_relation,fpb,"DB_player_relation",fpb.rpid)
		_net_mgr:sendPbMsgToPlayer(pdata.pid,_smsg.N_TROM_AGREE_ADD_FRIEND,fpb,"DB_player_relation")

	else
		r.type = RL_TYPE.RL_NONE
		self:sendRelation(pdata.pid)
		-- 删除
		self._realtion[pdata.pid] = nil
		_db_mgr:dbDeletePlayerData(self.pid,TABLE_INDEX.TAB_lp_player_relation,r,"DB_player_relation",r.rpid)
	end
end

function relation_module:onDeleteFriend(pdata)
	local pid = pdata.data
	local r = self:getRelation(pid)

	if r == nil then
		return
	end

	r.type = RL_TYPE.RL_NONE
	_db_mgr:dbDeletePlayerData(self.pid,TABLE_INDEX.TAB_lp_player_relation,r,"DB_player_relation",r.rpid)
	self:sendRelation(pid)
	self._realtion[pid] = nil
end

return relation_module