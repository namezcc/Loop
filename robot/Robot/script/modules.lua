MOD = {}
require "NetModule"
NetModule:init()

local mdname = {
    "LoginModule",
    "PlayerModule",
    "SendPackModule",
}

for i,v in ipairs(mdname) do
	local m = require(v)
    MOD[v] = m
	bind_player_func(m)
end

for k, v in pairs(MOD) do
	v:init()
end
