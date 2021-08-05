local Map = class()

function Map:ctor( cfg )
    self._cfg = cfg
    self._entity = {}
end

function Map:GetId()
    return self._cfg.id
end

function Map:AddEntity( ent )
    self._entity[ent:GetEntityId()] = ent
end

function Map:GetEntity( entId )
    return self._entity[entId]
end

function Map:RemoveEntity( entId )
    self._entity[entId] = nil
end

return Map