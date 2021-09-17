package.path = package.path..";./lua/room/?.lua;"

collectgarbage("setpause", 200)  --200
collectgarbage("setstepmul", 5000)

--------- load proto
pb.loadfile("./lua/room/proto/allproto.pb")

PLAYER_SOCK = {}
ALL_PLAYER = {}

require("class")

persent = require("util.persent")

require("log")
require("msg_define")
require("util.data_define")
require("util.table_index")
require("function")
require("util.pack")

require("msg_dispatch")


MOD = {}
local mod = require("lua_module")
local modlist = {}
for i,v in ipairs(mod.mgr_module) do
	local m = require("module."..v)
    MOD[v] = m
	modlist[#modlist+1] = m
end

for i,v in ipairs(mod.player_module) do
	local m = require("module."..v)
    MOD[v] = m
	modlist[#modlist+1] = m
	bind_player_func(m)
end

for i, v in ipairs(modlist) do
	v:init()
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

function update(dt)
	

end

print("load game main lua ...")
