local func = LFUNC
local SEND_SERVER_MSG_ID = LTOC.LTOC_SEND_SERVER
local _msgf = MSG_FUNC
local _room_mgr = ROOM_MGR_SERVER
local _pack = PACK
local _sermsg = SERVER_MSG

local net_mgr_module = {}

function net_mgr_module:init()
	self._ply_mgr = MOD.player_mgr_module

end

function net_mgr_module:sendToServer(ser,mid,pack)
	local path = {}

	path[1] = ser.type
	path[2] = ser.id

	func.CallCFunc(SEND_SERVER_MSG_ID,path,mid,pack:buff())
end

function net_mgr_module:sendToPathServer(path,mid,pack)
	func.CallCFunc(SEND_SERVER_MSG_ID,path,mid,pack:buff())
end

function net_mgr_module:sendPbMsgToPlayer(pid,mid,pb,proto)
	
	local player = self._ply_mgr:getPlayerByPid(pid)
	if player then
		_msgf.trans_msg_to_player(player,mid,pb)
	else
		local pack = _pack.buffpack()
		pack:writeint64(pid)
		pack:writeint32(mid)
		pack:writepb(pb,"LPMsg."..proto)
		self:sendToServer(_room_mgr,_sermsg.N_TRMM_TRANS_MSG_TO_PLAYER,pack)
	end
end

return net_mgr_module