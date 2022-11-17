local net_module = {}
local func = LFUNC
local _pack = PACK
local SEND_MSG_ID = LTOC.LTOC_SEND_MSG
local _net_mgr = nil

function net_module:init()
	
	_net_mgr = MOD.net_mgr_module

end

function net_module:sendMsg(mid,pb,pbname)
	pbname = "LPMsg."..pbname

	local buff = _pack.buffpack()
	buff:writepb(pb,pbname)

	func.CallCFunc(SEND_MSG_ID,self.sock,mid,buff:buff())
end

function net_module:sendToServer(ser,mid,pack)
	_net_mgr:sendToServer(ser,mid,pack)
end

return net_module