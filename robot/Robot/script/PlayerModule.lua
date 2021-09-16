local PlayerModule = {}
local _Net = NetModule

function PlayerModule:init()

	_Net:AddMsgCallBack(SMCODE.SM_RELATION_INFO,self.onRelationInfo,"PlayerSimpleInfoList")
	_Net:AddMsgCallBack(SMCODE.SM_TEAM_INFO,self.onTeamInfo,"TeamInfo")
	_Net:AddMsgCallBack(SMCODE.SM_TEAM_ASK_JOIN,self.onTeamAskJoinInfo,"PlayerSimpleInfo")
	
	
	-----------------
	CMD:AddCmdCall(self.DoSetPrint,"p","pam: : set print")
	CMD:AddCmdCall(self.ShowMid,"pmid","show msg id")
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

function PlayerModule:onRelationInfo(data)
	if not IS_SINGLE then
		return
	end

	print("relation info ...")
	printTable(data)
end

function PlayerModule:onTeamInfo(data)
	print("team info ---")
	printTable(data)
end

function PlayerModule:onTeamAskJoinInfo(data)
	print("team ask join ...")
	printTable(data)
	self._ask_pid = data.pid
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

------------------------------------------




return PlayerModule