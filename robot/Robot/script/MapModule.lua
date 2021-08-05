local Map = require "Map"
local MapModule = class()

function MapModule:ctor()
    self._map = nil
end

function MapModule:init(mods)
    
end

function MapModule:afterInit()
    
    


end

function MapModule:CreateMap( cfg )
    local map = Map.new(cfg)
    self._map = map
end

function MapModule:ChangeMap( mapid )
    if self._map then
        -- clear map ...
        self._map = nil
    end

    local cfg = CFG.GetMap(mapid)
    if cfg == nil then
        log_error("error map cfg=nil ",mapid)
        return
    end
    self:CreateMap(cfg)
    return self._map
end

function MapModule:GetMap()
    return self._map
end

function MapModule:AddPlayer( ply )
    self._map:AddEntity(ply)
end

return MapModule