local Scene = class()
require "ProDefine"
local Player = require "Player"
local AoiBlock = require "AoiBlock"

local MAX_PLAYER_INDEX = 40

function Scene:ctor(sid)
    self._sceneId = sid
    self._aoi = AoiBlock.new(100000,100000,10000,10000)
end

function Scene:Init()
    self._frame = 0
    self._playerIndex = 0
    self._player = {}
    self._aoiUpdate = {}
end

function Scene:Update(dt)
    for k,v in pairs(self._player) do
        v:Update(dt)
    end
end

function Scene:AddPlayer(pdata)
    local ply = Player.new(pdata)
    ply._id = self._playerIndex
    ply._scene = self
    self._playerIndex = self._playerIndex + 1
    self:AddToScene(ply)
end

function Scene:AddToScene(player)
    player._x = 0
    player._y = 0
    self._player[player._id] = player
    -- print("AddToScene")
    self._aoi:AddEntity(player)
end

function Scene:PlayerEnterControl(pid)
    local ply = self._player[pid]
    if ply == nil then
        print("not find player",pid)
        return
    end

    local _list = self._aoi:GetViewEntityPB(pid)

    local pb = {
        sceneid = self._sceneId,
        views = {pid},
        list = _list,   
    }
    --print("Send data")
    --print(persent.block(pb))
    NetModule:SendData("EntityList",SEND_MSG.N_ACK_SELF_ROLE_INFO,pb)
end

function Scene:RunFrame(opts)
    if opts.list then
        for i,opt in ipairs(opts.list) do
            local ply = self._player[opt.objId]
            if ply then
                ply:DoOperation(opt.list)
            end
        end
    end
    self:Update(NOW_TIME_DT)
    self:UpdateAoi()
    self._frame = self._frame + 1
    -- if self._frame % 60 == 0 then
    --     print("frame:",self._frame)
    -- end
end

function Scene:GetEntityInfo()
    if self._entpb == nil then
        self._entpb = {}
    end
    local _list = self._entpb

    for k,v in pairs(self._aoi._entity) do
        local _view = {}
        if v._ent:GetBroad() then
            table.insert(_view,k)
            for vid,_1 in pairs(v._view) do
                table.insert(_view,vid)
            end
        else
            for vid,_1 in pairs(v._enter) do
                table.insert(_view,vid)
            end
        end

        if #_view > 0 then
            local entpb = v._ent:GetInfoPB()
            local aoient = {
                views = _view,
                entity = entpb,
            }
            table.insert(_list,aoient)
        end
    end

    self._aoi:ClearView()

    if #_list > 0 then
        local info = {
            sceneid = self._sceneId,
            list = _list
        }
        self._entpb = nil
        return info
    end
end

function Scene:AddUpdateAoi(pid)
    table.insert(self._aoiUpdate,pid)
end

function Scene:UpdateAoi()
    for i,v in ipairs(self._aoiUpdate) do
        self._aoi:UpdateEntity(v)
    end
    if #self._aoiUpdate > 0 then
        self._aoiUpdate = {}
    end
end

return Scene