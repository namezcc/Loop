local Entity = require "Entity"
local Player = class("Player",Entity)

function Player:ctor(pdata)
    Player.super.ctor(self)
    self._name = pdata.name
    self._optbuff = {}
    self._broad = false
    self:SetProp(PROP_TYPE.PT_HP,100)
end

function Player:GetBroad()
    return self._broad
end

function Player:Update(dt)
    local index = self._scene._frame % OPT_BUFF_SIZE
    local opt = self._optbuff[index]
    if opt and opt.frame == self._scene._frame then
        for i,v in ipairs(opt.opts) do
            self:DoCmd(v)
        end
    end

    -- ...



end

function Player:DoOperation(opts)
    for i,v in ipairs(opts) do
        if v.frame >= self._scene._frame then
            local index = v.frame % OPT_BUFF_SIZE
            self._optbuff[index] = v
        end
    end
end

function Player:DoCmd(cmd)
    if cmd.type == OPT_TYPE.OPT_TEST then
        self:SetProp(PROP_TYPE.PT_HP,cmd.value[1])
        self._broad = true
    end
end

function Player:GetInfoPB()
    self._broad = false
    local info = {
        id = self._id,
        frame = self._scene._frame,
        props = self:GetPropPB(),
    }
    return info
end

return Player