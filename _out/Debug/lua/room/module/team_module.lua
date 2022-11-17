local team_module = {}
local _team_server_path = {}
local _net_mgr = nil
local _msg_func = MSG_FUNC
local _pack = PACK
local _smsg = SERVER_MSG
local _cm = CM
local _sm = SM
local _log = LOG

function team_module:init()
	
	_net_mgr = MOD.net_mgr_module

	_team_server_path[1] = SERVER_TYPE.ST_TEAM_PROXY
	_team_server_path[2] = 0
	_team_server_path[3] = SERVER_TYPE.ST_TEAM
	_team_server_path[4] = 0


	_msg_func.bind_player_proto_func(_cm.CM_TEAM_CREATE,self.onCreateTeam,"clientEmpty")
	_msg_func.bind_player_proto_func(_cm.CM_TEAM_DISBAND,self.onTeamDisband,"clientEmpty")
	_msg_func.bind_player_proto_func(_cm.CM_TEAM_QUIT,self.onTeamQuit,"clientEmpty")
	_msg_func.bind_player_proto_func(_cm.CM_TEAM_ASK_JOIN,self.onTeamAskJoin,"propertyInt64")
	_msg_func.bind_player_proto_func(_cm.CM_TEAM_REPLY_JOIN,self.onTeamReplyJoin,"ReplyTeamJoin")

	_msg_func.bind_player_server_msg(_smsg.N_TROM_TEAM_INFO,self.onUpdateTeamInfo)
	_msg_func.bind_player_server_msg(_smsg.N_TROM_TEAM_DISBAND,self.onGetTeamDisband)

	_msg_func.bind_player_server_proto(_smsg.N_TROM_TEAM_ASK_JOIN,self.onTeamGetAskJoin,"PlayerSimpleInfo")
	_msg_func.bind_player_server_proto(_smsg.N_TROM_TEAM_GET_REPLY_ASK_JOIN,self.onTeamGetReplyAskJoin,"TeamNode")
end

-- 登出处理
function team_module:logout()
	
	_log.warn("team logout")
	self:onTeamQuit()
end

function team_module:getTeamServerPath(teamsid)
	_team_server_path[4] = teamsid
	return _team_server_path
end

function team_module:removeTeam()
	self._team_id = nil
	self._team_leader = nil
	self._team_server_id = nil
	self._team_player = nil
end

function team_module:onCreateTeam(pdata)
	if self._team_id then
		return
	end

	print("create team ...")

	local pack = _pack.buffpack()
	pack:writeint32(SELF_SERVER.id)
	pack:writeint64(self.pid)
	pack:writestring(self.player.name)
	pack:writeint32(self.player.level)

	local serpath = self:getTeamServerPath(0)
	_net_mgr:sendToPathServer(serpath,_smsg.N_TEAM_CREATE_TEAM,pack)
end

function team_module:onUpdateTeamInfo(pack)
	
	self._team_server_id = pack:readint32()
	self._team_id = pack:readint32()
	self._team_leader = pack:readint64()

	local num = pack:readint32()

	self._team_player = {}

	local pbplayer = {}

	for i = 1, num do
		local info = {
			gameid = pack:readint32(),
			pid = pack:readint64(),
			name = pack:readstring(),
			level = pack:readint32(),
		}

		self._team_player[info.pid] = info

		pbplayer[i] = info
	end

	local pb = {
		leader = self._team_leader,
		player = pbplayer,
	}

	print("on update team info")
	printTable(pb)

	self:sendMsg(_sm.SM_TEAM_INFO,pb,"TeamInfo")
end

function team_module:onTeamDisband()
	if self._team_leader ~= self.pid then
		return
	end	

	local pack = _pack.buffpack()
	pack:writeint32(self._team_id)
	pack:writeint64(self.pid)

	local path = self:getTeamServerPath(self._team_server_id)
	_net_mgr:sendToPathServer(path,_smsg.N_TEAM_DISBAND_TEAM,pack)
end

function team_module:onGetTeamDisband(pack)

	_log.warn("team disband pid:%d",self.pid)

	self:removeTeam()
end

function team_module:onTeamQuit()
	if self._team_id == nil then
		return
	end

	local pack = _pack.buffpack()
	pack:writeint32(self._team_id)
	pack:writeint64(self.pid)

	local path = self:getTeamServerPath(self._team_server_id)
	_net_mgr:sendToPathServer(path,_smsg.N_TEAM_QUIT_TEAM,pack)
	self:removeTeam()
end

function team_module:onTeamAskJoin(pb)
	if self._team_id ~= nil then
		return
	end

	local msg = {
		pid = self.pid,
		name = self.player.name,
		level = self.level,
	}
	_net_mgr:sendPbMsgToPlayer(pb.data,_smsg.N_TROM_TEAM_ASK_JOIN,msg,"PlayerSimpleInfo")
end

function team_module:onTeamGetAskJoin(pb)
	if self._team_leader ~= self.pid then
		return
	end

	self:sendMsg(_sm.SM_TEAM_ASK_JOIN,pb,"PlayerSimpleInfo")
end

function team_module:onTeamReplyJoin(pb)
	if pb.res == 0 then
		return
	end

	if self._team_leader ~= self.pid then
		return
	end

	local msg = {
		team_sid = self._team_server_id,
		team_id = self._team_id,
	}
	_net_mgr:sendPbMsgToPlayer(pb.pid,_smsg.N_TROM_TEAM_GET_REPLY_ASK_JOIN,msg,"TeamNode")
end

function team_module:onTeamGetReplyAskJoin(pb)
	local pack = _pack.buffpack()
	pack:writeint32(SELF_SERVER.id)
	pack:writeint32(pb.team_id)
	pack:writeint64(self.pid)
	pack:writestring(self.player.name)
	pack:writeint32(self.player.level)

	local path = self:getTeamServerPath(pb.team_sid)
	_net_mgr:sendToPathServer(path,_smsg.N_TEAM_JOIN_TEAM,pack)
end

return team_module