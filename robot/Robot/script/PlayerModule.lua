local PlayerModule = {}
local _Net = NetModule

function PlayerModule:init()

	_Net:AddMsgCallBack(SMCODE.SM_PLAYER_ALL_INFO,self.onShowData,"SmPlayerAllMsg","SM_PLAYER_ALL_INFO")
	_Net:AddMsgCallBack(SMCODE.SM_PLAYER_INFO,self.onShowData,"SmPlayerInfo","SM_PLAYER_INFO")
	
	-----------------
	CMD:AddCmdCall(self.ShowMid,"pmid","show msg id")
	CMD:AddCmdCall(self.doDebug,"db","send cmd debug")
	CMD:AddCmdCall(self.DoSerchPlayer,"sp","uid: serch player...")
	CMD:AddCmdCall(self.doAddFriend,"fr","uid: add friend...")
	CMD:AddCmdCall(self.doApplyFriend,"rfr","uid res : reply friend...")
	CMD:AddCmdCall(self.doDeleteFriend,"df","uid : delete friend...")
	CMD:AddCmdCall(self.doCreateTeam,"team","create team")
	CMD:AddCmdCall(self.doTeamDisband,"teamd","disband team")
	CMD:AddCmdCall(self.doTeamQuit,"teamq","quit team")
	CMD:AddCmdCall(self.doTeamAskJoin,"teamj","pid : ask join team")
	CMD:AddCmdCall(self.doTeamReplyJoin,"teamr","res 0 or 1 : reply ask join team")
end

---------------------- msg ---------------------------------

function PlayerModule:OnShowMsgInfo( data,msg )
	print(msg)
	printTable(data)
end

function PlayerModule:onShowData(data,ext,pbstr)
	if not IS_SINGLE then
		return
	end
	print("")
	print(ext or "\t","\t",pbstr)
	printTable(data)
end

----------------  CMD ---------------------

local TN = tonumber

function PlayerModule:ShowMid( pam )
	local mid = TN(pam[1])
	if mid == 0 then
		if SHOW_MID == true then
			SHOW_MID = false
		else
			SHOW_MID = true
		end
	else
		print("get msg id ",mid,"=",MID_MAP[mid])
	end
end

function PlayerModule:Relogin( )
	self:DoConnectServer()
end

function PlayerModule:DoSerchPlayer(pam)
	local pid = TN(pam[1])

	local pb = {
		data = pid
	}

	print("seach ",pb.data)

	self:SendGameData("propertyInt64",CMCODE.CM_SEARCH_PLAYER,pb)
end

local CMD_TYPE = {
	level = 1,
}

function PlayerModule:doDebug(pam)
	local data = {}

	for i, v in ipairs(pam) do
		if i == 1 then
			data[i] = CMD_TYPE[v]
		else
			data[i] = TN(v)
		end
	end

	self:sendMsg(CMCODE.CM_DEBUG,{data=data},"Int32Array")
end

function PlayerModule:doAddFriend(pam)
	local pid = TN(pam[1])

	local pb = {
		data = pid
	}

	self:sendMsg(CMCODE.CM_ADD_FRIEND,pb,"propertyInt64")
end

function PlayerModule:doApplyFriend(pam)
	local r = TN(pam[1])

	local pb = {
		pid = r,
		res = TN(pam[2]),
	}
	self:sendMsg(CMCODE.CM_OPT_FRIEND_APPLY,pb,"ReplyFriend")
end

function PlayerModule:doDeleteFriend(pam)
	local pid = TN(pam[1])

	local pb = {
		data = pid
	}

	self:sendMsg(CMCODE.CM_DELETE_FRIEND,pb,"propertyInt64")
end

function PlayerModule:doCreateTeam()
	
	self:sendMsg(CMCODE.CM_TEAM_CREATE,{},"clientEmpty")
end

function PlayerModule:doTeamDisband()
	self:sendMsg(CMCODE.CM_TEAM_DISBAND,{},"clientEmpty")
end

function PlayerModule:doTeamQuit()
	self:sendMsg(CMCODE.CM_TEAM_QUIT,{},"clientEmpty")
end

function PlayerModule:doTeamAskJoin(pam)
	local pb = {
		data = TN(pam[1])
	}

	self:sendMsg(CMCODE.CM_TEAM_ASK_JOIN,pb,"propertyInt64")
end

function PlayerModule:doTeamReplyJoin(pam)
	local pb = {
		pid = self._ask_pid,
		res = TN(pam[1])
	}
	self:sendMsg(CMCODE.CM_TEAM_REPLY_JOIN,pb,"ReplyTeamJoin")
end

function PlayerModule:sendMsg(mid,pb,proto)
	self:SendGameData(proto,mid,pb)
end

function PlayerModule:doSendMsg(pam,ext)
	local proc = ext[1]
	local proto = ext[2]
	local ischat = ext[3] or false

	local pb = {}

	if proto == "Int32Value" then
		pb.value = TN(pam[1])
	elseif proto == "ProtoInt32Array" then
		local vec = {}
		for i, v in ipairs(pam) do
			vec[#vec+1] = TN(v)
		end
		pb.i32 = vec
	elseif proto == "ProtoPairInt32Array" then
		local vec = {}
		for i = 1, #pam, 2 do
			vec[#vec+1] = {
				data1 = TN(pam[i]),
				data2 = TN(pam[i+1]),
			}
		end
		pb.list = vec
	elseif proto == "PInt64" then
		pb.data = TN(pam[1])
	elseif proto == "PInt64Array" then
		local vec = {}
		for i, v in ipairs(pam) do
			vec[#vec+1] = TN(v)
		end
		pb.data = vec
	elseif proto == "PInt64Pair32Array" then
		pb.i64 = TN(pam[1])
		local vec = {}
		for i = 2, #pam,2 do
			vec[#vec+1] = {
				data1 = TN(pam[i]),
				data2 = TN(pam[i+1]),
			}
		end
		pb.array = vec
	elseif proto == "VString" then
		pb.val = pam[1]
	end
	self:sendMsg(proc,pb,proto)
end

------------------------------------------




return PlayerModule