package.path = package.path..";./script/?.lua;"

LoopState = CLuaState

TPB = require "pb"
assert(TPB.loadfile("./proto/allproto.pb"))

require "function"
require "Log"
require "class"
persent = require "persent"
require "schedule"
pbparse = require "parse"
require "eventManager"
require "EventDefine"
require "Math"
require "GameDefine"
require "CfgData"
CMD = require "CmdManager"

pbparse:SetProtoPath("../../proto/client/")
pbparse:ParseFile("define.proto")

CMCODE = pbparse:GetMessage("LP_CM_MSG_ID")
SMCODE = pbparse:GetMessage("LP_SM_MSG_ID")


require "modules"

-- printTable(CMCODE)

GAME_INDEX = 0
function GameInit(index)
    GAME_INDEX = index
end

local games = {}
local nid = 1
local checkNum = 500
local function CreateGame()

	local account = "zcc"..(GAME_INDEX*checkNum+nid)
	local player = {}

    games[nid] = player
	setPlayerImpFunc(player)

	if IS_SINGLE and GAME_ADDR[GAME_INDEX] and GAME_ADDR[GAME_INDEX].uid then
		account = GAME_ADDR[GAME_INDEX].uid
	end

	player:onNewPlayer(account)

    nid = nid + 1
end

if checkNum == 1 then
    GameType = GAME_TYPE.SINGLE     --单人 模式
	IS_SINGLE = true
else
    GameType = GAME_TYPE.MUILT
end

--  发包器模式
-- GameType = GAME_TYPE.SEND_PACK

if GameType == GAME_TYPE.SEND_PACK then
    -- GAME_INDEX = 2
end

collectgarbage("setpause",200)  --200
collectgarbage("setstepmul",5000)


DELAY_START = 3000

NOW_TICK = 0
BEG_TIME = 0
NOW_SEC = 0

ADD_INT = 1

FINISH_TABLE = {}

function Main()
    print("start Main")



end

function getMillisecond()
	return LoopState:callFunction("GetMilliSecend")
end

function UpdateGame( dt )
    if NOW_TICK == 0 then
        BEG_TIME = dt
        NOW_SEC = dt + ADD_INT
        -- print("began time -------->",dt)
    end
    NOW_TICK = dt

	if DELAY_START > 0 then
		if NOW_TICK - BEG_TIME < DELAY_START then
			return
		end
		BEG_TIME = dt
		DELAY_START = 0
	end

    if nid <= checkNum and NOW_SEC < dt then
        CreateGame()
        NOW_SEC = dt + ADD_INT
    end

	Schedule:Run(dt)
end

function SplitPam( pam )
    local tab = {}
    local index = 0
    local first = nil
    local cmdname = nil
    for s in string.gmatch( pam,"[^%s]+") do
        if index == 0 then
			first = s
			cmdname = s
        elseif index == 1 then
            cmdname = s
            if GameType == GAME_TYPE.SINGLE then
                table.insert( tab,s )
                cmdname = first
            end
        else
            table.insert( tab,s )
        end
        index = index + 1
    end
    return first,cmdname,tab
end

function DoCmdFunction( pamstr )
	if GameType == GAME_TYPE.SEND_PACK then
		local g = games[1]
		g._M.SendPackModule:SendPack(pamstr)
		return
	end

    local first,cmdname,cmd = SplitPam(pamstr)
    if first == "h" then
        CMD:ShowHelp(cmd[1])
        return
    end

    local gid = 1

    if GameType ~= GAME_TYPE.SINGLE then
        gid = tonumber(first)
    end

    if gid == nil then
        print("error game id")
        return
    end

    if gid == 0 then
        for i,v in ipairs(games) do
            CMD:DoCmd(cmdname,v,cmd)
        end
    else
        local game = games[gid]
        if player == nil then
            print("error player id")
            return
        end
        CMD:DoCmd(cmdname,player,cmd)
    end
end