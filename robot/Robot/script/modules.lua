GM_MODULE = {}
local mdname = {
    "LoginModule",
    "NetModule",
    "PlayerModule",
    "MapModule",
    "SendPackModule",
}

for i,v in ipairs(mdname) do
    GM_MODULE[v] = require(v)
    GM_MODULE[v].MOD_NAME = v
end