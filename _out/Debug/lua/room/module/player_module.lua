local _func = LFUNC
local _dopt = DATA_OPT
local _table = TABLE_INDEX
local _syncf = SYNC_FUNC
local _client_sync = SYNC_CLIENT_FUNC
local _save_cash = SAVE_CASH
local _sql_opt = SQL_OPRATION

local getTable = table.getOrNewTable

local _net = nil
local _db_mgr = nil


local player_module = {}

function player_module:init()
	_net = MOD.net_mgr_module
	_db_mgr = MOD.db_mgr_module


	MSG_FUNC.bind_player_proto_func(CM.CM_DEBUG,self.onDebug,"Int32Array")
end

function player_module:initDB(data)
	self._player_info = data.player
	self._player_info.login_time = _func.getNowSecond()

	self._client_sync = {}
end

function player_module:sendClienMsg(pbmsg)
	pbmsg.player_info = self._player_info
end

function player_module:logout()
	self._player_info.logout_time = _func.getNowSecond()
	self:updateData(TABLE_INDEX.TAB_lp_player,self._player_info,true)
	print("logout --- ")
end

function player_module:sendMsg(mid,pb,pbname)
	_net:sendPbToPlayer(self,mid,pb,pbname)
end
---@param tab table table_index
---@param data table data
---@param nocash boolean 是否立刻更新
function player_module:insertData(tab,data,nocash)
	if nocash then
		self:sendDataDbOpt({_table=tab,_opt = _dopt.DO_INSERT,_data = data})
	else
		local cash = getTable(_save_cash,self.cid)
		cash[#cash+1] = {_table=tab,_opt = _dopt.DO_INSERT,_data = data}
	end
end

---@param tab table table_index
---@param data table data
---@param nocash boolean 是否立刻更新
function player_module:updateData(tab,data,nocash)
	if nocash then
		self:sendDataDbOpt({_table=tab,_opt = _dopt.DO_UPDATE,_data = data})
	else
		local cash = getTable(_save_cash,self.cid)
		cash[#cash+1] = {_table=tab,_opt = _dopt.DO_UPDATE,_data = data}
	end
end

---@param tab table table_index
---@param data table data
---@param nocash boolean 是否立刻更新
function player_module:deleteData(tab,data,nocash)
	if nocash then
		self:sendDataDbOpt({_table=tab,_opt = _dopt.DO_DELETE,_data = data})
	else
		local cash = getTable(_save_cash,self.cid)
		cash[#cash+1] = {_table=tab,_opt = _dopt.DO_DELETE,_data = data}
	end
end

function player_module:clientUpdateData(tab,data,isvec)
	if isvec then
		local svec = getTable(self._client_sync,tab)
		svec[#svec+1] = {_data=data,_opt=_dopt.DO_UPDATE}
	else
		self._client_sync[tab] = data
	end
end

function player_module:clientDeleteData(tab,data)
	local svec = getTable(self._client_sync,tab)
	svec[#svec+1] = {_data=data,_opt=_dopt.DO_DELETE}
end

function player_module:syncData()
	local cash = _save_cash[self.cid]
	if cash == nil or #cash == 0 then
		return
	end

	local dbvec = {}
	local savepb = {
		cid = self.cid,
		datas = dbvec,
	}

	for i, v in ipairs(cash) do
		_syncf[v._table](self,v._table,v._opt,v._data,dbvec)
	end
	if #dbvec > 0 then
		_db_mgr:sendDbOptProto(self.cid,_sql_opt.SOP_UPDATE_PLAYER_DATA,savepb,"DB_sql_data_list")
	end
	_save_cash[self.cid] = nil
	self:syncClientData()
end

function player_module:sendDataDbOpt(cash)
	local dbvec = {}
	local savepb = {
		cid = self.cid,
		datas = dbvec,
	}
	_syncf[cash._table](self,cash._table,cash._opt,cash._data,dbvec)
	if #dbvec > 0 then
		_db_mgr:sendDbOptProto(self.cid,_sql_opt.SOP_UPDATE_PLAYER_DATA,savepb,"DB_sql_data_list")
	end
end

function player_module:syncClientData()
	if table.empty(self._client_sync) then
		return
	end

	for tab, v in pairs(self._client_sync) do
		local f = _client_sync[tab]
		if f then
			f(self,v)
		end
	end
	self._client_sync = {}
end

local DEBUG_TYPE = {
	SET_LEVEL = 1
}

function player_module:onDebug(data)
	local type = data.data[1]

	if type == DEBUG_TYPE.SET_LEVEL then
		local lv = data.data[2]
		self:updateData(_table.TAB_lp_player,{level=lv})
	end

	self:syncData()
end

return player_module