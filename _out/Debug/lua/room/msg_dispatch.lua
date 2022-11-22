local _log = LOG
local _pbdecode = pb.decode
local msg_func_tab = {}
-- 服务器->本地玩家消息
local trans_msg_func_tab = {}			
local func_tab = {}
local all_player = ALL_PLAYER
local pack = PACK

local function bind_mod_proto_func(mid,mod,f,proto)
	local _pro = "LPMsg."..proto
	msg_func_tab[mid] = function (netbuf,buffpack)
		local mdata = _pbdecode(_pro,netbuf)
		if mdata then
			f(mod,mdata)
		end
	end
end

local function bind_player_proto_func(mid,f,proto)
	local _pro = "LPMsg."..proto
	msg_func_tab[mid] = function (uid,netbuf)
		local ply = all_player[uid]
		if ply == nil then
			_log.error("no player in lua uid:%d",uid)
			return
		end
		local mdata = _pbdecode(_pro,netbuf)
		if mdata then
			f(ply,mdata)
		end
	end

end

local function bind_player_server_msg(mid,f)
	local ply_mgr = MOD.player_mgr_module
	msg_func_tab[mid] = function (netbuf,netmsg)
		local netpack = pack.netpack(netmsg)
		local cid = netpack:readint32()
		local player = ply_mgr:getPlayerByCid(cid)

		if player then
			f(player,netpack)
		else
			_log.error("on player server msg player nil mid:%d pid:%d",mid,cid)
		end
	end
end

local function bind_player_server_proto(mid,f,proto)
	local ply_mgr = MOD.player_mgr_module
	local _pro = "LPMsg."..proto
	msg_func_tab[mid] = function (netbuf,netmsg)
		local netpack = pack.netpack(netmsg)
		local cid = netpack:readint32()
		local player = ply_mgr:getPlayerByCid(cid)
		local mdata = netpack:decode(_pro)

		if player then
			if mdata then
				f(player,mdata)
			end
		else
			_log.error("on player server proto player nil mid:%d pid:%d",mid,cid)
		end
	end

	trans_msg_func_tab[mid] = f
end

local function bind_mod_pack_func(mid,mod,f)
	msg_func_tab[mid] = function (netbuf,msg)
		local netpack = pack.netpack(msg)
		f(mod,netpack)
	end
end

local function bind_mod_func(mid,mod,f)
	func_tab[mid] = function (...)
		f(mod,...)
	end
end

local function onNetMsg(mid,...)
	local f = msg_func_tab[mid]
	if f then
		f(...)
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