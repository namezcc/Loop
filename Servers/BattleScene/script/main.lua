
local scriptPath = LoopState:callFunction("GetScriptPath")
local protoPath = LoopState:callFunction("GetProtoPath")

package.path = package.path..";"..scriptPath.."?.lua;"

print("scriptPath",scriptPath)
print("protoPath",protoPath)
print("load main lua")

require "print"
pbparse = require "parse"
persent = require "persent"
pbparse:SetProtoPath(protoPath)

pbparse:ParseFile("server/common.proto")
pbparse:ParseFile("server/sroom.proto")
pbparse:ParseFile("server/sbattle.proto")
pbparse:ParseFile("client/battle.proto")

require "GameDefine"
require "NetModule"
require "SceneModule"

collectgarbage("setpause",200)  --200
collectgarbage("setstepmul",5000)

function Main()
    SceneModule:Init()
end

NOW_TIME_DT = 0

function MainUpdate(dt)
    NOW_TIME_DT = dt
    --SceneModule:Update(dt)
end

function ClearGC()
    


end