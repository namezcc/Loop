CFG = {
}
local cfgfile = {
    "map",
    "attr",
}

local initFunc = {}
initFunc.attr = function(data)
    local ndata = {}
    for k,v in pairs(data) do
        ndata[v.attr_type] = v
    end
    CFG["attr"] = ndata
end

for i,v in ipairs(cfgfile) do
    CFG[v] = require("cfgdata."..v)
    if initFunc[v] then
        initFunc[v](CFG[v])
    end
end

function CFG.GetMap(id)
    return CFG["map"][id]
end

function CFG.GetAttr( id )
    return CFG["attr"][id]
end

