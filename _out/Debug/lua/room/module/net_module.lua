local net_module = {}
local func = LFUNC
local _pack = PACK
local SEND_MSG_ID = LTOC.LTOC_SEND_MSG
local _net_mgr = nil

function net_module:init()
	
	_net_mgr = MOD.net_mgr_module

end

function net_module:sendToServer(ser,mid,pack)
	_net_mgr:sendToServer(ser,mid,pack)
end

return net_module