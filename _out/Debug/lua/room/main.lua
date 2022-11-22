package.path = package.path..";lua/room/?.lua;"

collectgarbage("setpause", 200)  --200
collectgarbage("setstepmul", 5000)

--------- load proto
pb.loadfile("lua/room/proto/allproto.pb")

PLAYER_GATE = {}
ALL_PLAYER = {}
SAVE_CASH = {}		-- 保存缓存

require("class")

persent = require("util.persent")

require("log")
require("msg_define")
require("util.data_define")
require("util.table_index")
require("function")
require("util.pack")
require("msg_dispatch")
require("util.sync_func")
require("util.sync_client_func")


MOD = {}
local mod_name = require("lua_module")
local modmgr = {}
local modplayer = {}
local mgrupdatemod = {}

function initModule()
	for i,v in ipairs(mod_name.mgr_module) do
		local m = require("module."..v)
		m.name = v
		MOD[v] = m
		modmgr[#modmgr+1] = m
	
		if m.update then
			mgrupdatemod[#mgrupdatemod+1] = m
		end
	end
	
	for i,v in ipairs(mod_name.player_module) do
		local m = require("module."..v)
		m.name = v
		MOD[v] = m
		modplayer[#modplayer+1] = m
		bind_player_func(m,v)
	end
	
	for i, v in ipairs(modmgr) do
		if v.init == nil then
			LOG.error("mod %s need init func",v.name)
		else
			v:init()
		end
	end
	
	for i, v in ipairs(modmgr) do
		if v.init_func == nil then
			LOG.error("mgr %s mod need init_func",v.name)
		else
			v:init_func()
		end
	end
	
	for i, v in ipairs(modplayer) do
		if v.init == nil then
			LOG.error("mod %s need init func",v.name)
		else
			v:init()
		end
	end
end

function ReloadModule( module_name )
    local old_module = package.loaded[module_name] or {}

    package.loaded[module_name] = nil
    require (module_name)

    local new_module = package.loaded[module_name]
    for k, v in pairs(new_module) do
        old_module[k] = v
    end

    package.loaded[module_name] = old_module
    return old_module
end

function HotLoadModule( )
    package.loaded["HotLoad"] = nil
    require("HotLoad")
end

function gc_collect()
	LOG.info("befor gc %f k",collectgarbage("count"))
	collectgarbage("collect")
	LOG.info("after gc %f k",collectgarbage("count"))
end


-- update
local _time = TIME_DATA
function onFrameUpdate(tick,now)
	_time.TICK_TIME_STAMEP = tick
	local old = _time.TIME_STAMEP
	_time.TIME_STAMEP = now

	if old ~= now then
		for i, m in ipairs(mgrupdatemod) do
			m:update(now)
		end
	end

end
BindLuaFunc(CTOL.CTOL_FRAME_UPDATE,onFrameUpdate)

function main(id)
	_time.TIME_STAMEP = os.time()
	_time.TICK_TIME_STAMEP = os.time()*1000

	SELF_SERVER.id = id

	initModule()
	math.randomseed(tostring(os.time()):reverse():sub(1, 6))
end

-- lua报错之后的处理,清理一些东西
function luaErrorCall()
	if not table.empty(SAVE_CASH) then
		SAVE_CASH = {}
		print("clear save cash")
	end
end

print("load game main lua ...")
