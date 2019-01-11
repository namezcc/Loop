local Entity = require "Entity"
local Player = class("Player",Entity)

function Player:ctor(pdata)
    Player.super.ctor(self)
    self._name = pdata.name
    self._optbuff = {}
    self._broad = false
    self._moveKeepFrame = 0
    self._speed = 10
    self:initAttr()
end

function Player:initAttr()
    self:SetProp(PROP_TYPE.PT_HP,100)
    self:SetProp(PROP_TYPE.PT_POS_X,0)
    self:SetProp(PROP_TYPE.PT_POS_Y,0)
    self:SetProp(PROP_TYPE.PT_V_X,0)
    self:SetProp(PROP_TYPE.PT_V_Y,0)
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
    self:UpdateMove()

end

function Player:UpdateMove()
    if self._moveKeepFrame <= 0 then
        return
    end

    self._moveKeepFrame = self._moveKeepFrame - 1

    self._x = self._x + self:GetProp(PROP_TYPE.PT_V_X)
    self._y = self._y + self:GetProp(PROP_TYPE.PT_V_Y)

    self:SetProp(PROP_TYPE.PT_POS_X,self._x)
    self:SetProp(PROP_TYPE.PT_POS_Y,self._y)

    self._scene:AddUpdateAoi(self._id)

    self._broad = true

    print(string.format("move frame:%d x:%d y:%d",self._scene._frame,self._x,self._y))
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
    self._broad = true
    local func = OPT_FUNC[cmd.type]
    if func then
        func(self,cmd)
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