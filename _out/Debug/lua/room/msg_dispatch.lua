local _log = LOG

local msg_func_tab = {}
-- 服务器->本地玩家消息
local trans_msg_func_tab = {}			
local func_tab = {}
local all_player = ALL_PLAYER
local pack = PACK

local function bind_mod_proto_func(mid,mod,f,proto)
	local _pro = "LPMsg."..proto
	msg_func_tab[mid] = function (sock,netmsg)
		local netpack = pack.netpack(netmsg)
		local mdata = netpack:decode(_pro)
		if mdata then
			f(mod,mdata)
		end
	end
end

local function bind_player_proto_func(mid,f,proto)
	local _pro = "LPMsg."..proto
	msg_func_tab[mid] = function (sock,netmsg)
		local ply = all_player[sock]
		if ply == nil then
			_log.error("no player in lua sock:%d",sock)
			return
		end
		local netpack = pack.netpack(netmsg)
		local mdata = netpack:decode(_pro)
		if mdata then
			f(ply,mdata)
		end
	end

end

local function bind_player_server_msg(mid,f)
	local ply_mgr = MOD.player_mgr_module
	msg_func_tab[mid] = function (sock,netmsg)
		local netpack = pack.netpack(netmsg)
		local pid = netpack:readint64()
		local player = ply_mgr:getPlayerByPid(pid)

		if player then
			f(player,netpack)
		else
			_log.error("on player server msg player nil mid:%d pid:%d",mid,pid)
		end
	end
end

local function bind_player_server_proto(mid,f,proto)
	local ply_mgr = MOD.player_mgr_module
	local _pro = "LPMsg."..proto
	msg_func_tab[mid] = function (sock,netmsg)
		local netpack = pack.netpack(netmsg)
		local pid = netpack:readint64()
		local player = ply_mgr:getPlayerByPid(pid)
		local mdata = netpack:decode(_pro)

		if player then
			if mdata then
				f(player,mdata)
			end
		else
			_log.error("on player server proto player nil mid:%d pid:%d",mid,pid)
		end
	end

	trans_msg_func_tab[mid] = f
end

local function bind_mod_pack_func(mid,mod,f)
	msg_func_tab[mid] = function (sock,msg)
		local netpack = pack.netpack(msg)
		f(mod,netpack)
	end
end

local function bind_mod_func(mid,mod,f)
	func_tab[mid] = function (...)
		f(mod,...)
	end
end

local function onNetMsg(mid,sock,netmsg)
	local f = msg_func_tab[mid]
	if f then
		f(sock,netmsg)
	else
		_log.error("netmsg not bind func %d",mid)
	end
end

local function onMsg(mid,...)
	local f = func_tab[mid]
	if f then
		f(...)
	else
		_log.error("cmsg not bind func:%d",mid)
	end
end

local function trans_msg_to_player(player,mid,msg)
	local f = trans_msg_func_tab[mid]

	if f == nil then
		_log.error("trans_msg_to_player f nil mid:%d",mid)
		return
	end

	f(player,msg)
end

BindLuaFunc(CTOL.CTOL_NET_MSG,onNetMsg)
BindLuaFunc(CTOL.CTOL_MSG,onMsg)

MSG_FUNC = {
	bind_mod_proto_func = bind_mod_proto_func,
	bind_mod_func = bind_mod_func,
	bind_player_proto_func = bind_player_proto_func,
	bind_mod_pack_func = bind_mod_pack_func,
	bind_player_server_msg = bind_player_server_msg,
	bind_player_server_proto = bind_player_server_proto,
	trans_msg_to_player = trans_msg_to_player,
}