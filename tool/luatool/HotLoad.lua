-- 热更逻辑

-- 热更的模块
local hotModule = {
	-- "GemModule",
	-- "BagModule",
	-- "PlayerModule",
	-- "TalentModule",
	-- "DbModule",
	-- "SoulModule",
}

for i,v in ipairs(hotModule) do
    local m = ReloadModule("module."..v)
    m:init()
    MOD[v] = m
end

-- cfg 热更

local hotCfg = {
	-- "yaoji",
}

for i,v in ipairs(hotCfg) do
	ReloadCfgData(v)
end

print("hot reload cfg")

-- 其他



-------------------
