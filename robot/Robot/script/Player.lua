local Entity = require "Entity"
local Player = class("Player",Entity)

function Player:ctor(entId,x,z)
    Player.super.ctor(self,entId,ENTITY_TYPE.ET_PLAYER)
    self:SetPos(x,z)

    
end

function Player:init( entId,x,z)
    self:SetEntityId(entId)
    self:SetPos(x,z)
end


return Player