SceneModule = {
    _cashScene = {},
    _runScene = {},
}

local MAX_SCENE_SIZE = 300
local Scene = require "Scene"

function SceneModule:Init()
    for i=1,MAX_SCENE_SIZE do
        local sce = Scene.new(i)
        sce:Init()
        self._cashScene[sce._sceneId] = sce
    end
    
    self._entInfo = {}

    NetModule:AddMsgCallBack(LPMSG.N_REQ_BEGIN_BATTLE_SCENE,self.OnReqBeginBattleScene,self,"Int32Value")
    NetModule:AddMsgCallBack(LPMSG.N_REQ_BATTLE_ADD_PLAYER,self.OnReqAddPlayer,self,"BatPlayerInfo")
    NetModule:AddMsgCallBack(LPMSG.N_REQ_ENTER_BATTLE_SCENE,self.OnPlayerEnterScene,self,"PlayerNode")
    NetModule:AddMsgCallBack(LPMSG.N_ACK_PLAYER_OPERATION,self.OnGetPlayerOperation,self,"SceneCmdList")
end

function SceneModule:Update(dt)
    for k,v in pairs(self._runScene) do
        v:Update(dt)
    end
end

function SceneModule:OnReqBeginBattleScene(data)
    print("BeginBattle Scene",data.value)
    --print(persent.block(data))
    local scene = self._cashScene[data.value]
    if scene then
        self._runScene[data.value] = scene
        self._cashScene[data.value] = nil

        LoopState:callFunction("RunScene",data.value)
    end
end

function SceneModule:OnReqAddPlayer(data)
    print("AddPlayer name:",data.name)
    --print(persent.block(data))
    local scene = self._runScene[data.sceneId]
    if scene then
        scene:AddPlayer(data)
    end
end

function SceneModule:OnPlayerEnterScene(data)
    print("OnPlayerEnterScene sceneId",data.proxyid,data.playerid)
    --print(persent.block(data))
    local scene = self._runScene[data.proxyid]
    if scene then
        scene:PlayerEnterControl(data.playerid)
    end
end

function SceneModule:OnGetPlayerOperation(data)
    local optscene = {}
    for i,v in ipairs(data.sceneCmd) do
        local scene = self._runScene[v.sceneid]
        if scene then
            table.insert(optscene,scene)
            scene:RunFrame(v)
        end
    end
    if #optscene > 0 then
        self:EntityUpdate(optscene)
    end
end

function SceneModule:EntityUpdate(runscene)
    for k,v in pairs(runscene) do
        local info = v:GetEntityInfo()
        if info then
            table.insert(self._entInfo,info)
        end
    end
    if #self._entInfo > 0 then
        local pb = {list = self._entInfo}
        NetModule:SendData("AoiSceneEntityList",SEND_MSG.N_ACK_OBJECT_INFO_ENTER_VIEW,pb)
        self._entInfo = {}
    end
end