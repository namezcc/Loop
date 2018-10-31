local BaseProp = class()

function BaseProp:GetValue()
end

function BaseProp:SetValue( ... )
end

local SingleProp = class("SingleProp",BaseProp)

function SingleProp:ctor(v)
    self._type = 1
    self._value = v
end

function SingleProp:GetValue()
    return self._value
end

function SingleProp:SetValue(v)
    self._value = v
end

local MultProp = class("MultProp",BaseProp)

function MultProp:ctor(vtab)
    self._type = 2
    self._value = vtab
end

function MultProp:GetValue()
    local nv = {}
    for k,v in pairs(self._value) do
        nv[k] = v
    end
    return nv
end

function MultProp:SetValue(vtab)
    self._value = vtab
end

local Entity = class()

function Entity:ctor()
    self._x = 0
    self._y = 0
    self._id = 0
    self._prop = {}
end

function Entity:SetProp(_type,v)
    self._prop[_type] = v
end

function Entity:GetProp(_type)
    return self._prop[_type]
end

function Entity:GetPropPB()
    local list = {}

    for k,v in pairs(self._prop) do
        local pb = {}
        pb.type = k
        pb.value = v
        table.insert(list,pb)
    end
    return list
end

function Entity:GetInfoPB()
    
end

return Entity