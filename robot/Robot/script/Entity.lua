local Entity = class()

function Entity:ctor(entId,etype)
    self._x = 0
    self._z = 0
    self._y = 0
    self._entId = entId
    self._type = etype

    self._attr = {}
end

function Entity:SetEntityId(id)
    self._entId = id
end

function Entity:GetEntityId()
    return self._entId
end

function Entity:GetType()
    return self._type
end

function Entity:SetPos( x,z )
    self._x = x
    self._z = z
end

function Entity:GetPos()
    return self._x,self._z
end

function Entity:SetMap( map )
    self._map = map
end

function Entity:RemoveMap()
    if self._map then
        self._map:RemoveEntity(self._entId)
        self._map = nil
    end
end

function Entity:SetAttr(_type,_val)
    self._attr[_type] = _val
end

function Entity:GetAttr( _type )
    return self._attr[_type] or 0
end

return Entity