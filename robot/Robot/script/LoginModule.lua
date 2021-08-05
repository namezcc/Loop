local LoginModule = class()

function LoginModule:ctor(account,pass)
    self._account = account
    self._pass = pass
end

function LoginModule:init(mods)
    self._M = mods
    self._Net = mods.NetModule

    -- self._M.Event:AddEvent(EventID.ON_GAME_INIT,function()
    --     self._M.Schedule:AddIntervalTask(function()
    --         self:DoConnectServer()
    --     end,0,1000,1)
    -- end)

    self._M.Event:AddEvent(EventID.ON_CONNECT_LOGIN_SERVER,self.OnConnect,self)
	self._M.Event:AddEvent(EventID.ON_CONNECT_ROOM_SERVER,self.OnConnectRoom,self)

end

function LoginModule:afterInit()

	self._Net:AddMsgCallBack(SMCODE.SM_LOGIN_RES,self.onLogin,self,"RoomInfo")
	self._Net:AddMsgCallBack(SMCODE.SM_SELF_ROLE_INFO,self.onGetRole,self,"RoleList")
	-- self._Net:AddMsgCallBack(SMCODE.SM_CLIENT_REPLY,self.OnCreateReply,self,"ServerMsgClientReply")
	-- self._Net:AddMsgCallBack(3888,self.OnTest,self,"ClientMsgLogin")

    CMD:AddCmdCall(self,self.DoCreateRole,"crole","pam:name : create role")
    CMD:AddCmdCall(self,self.DoEnterGame,"enter","pam:roleindex :enter game")
    CMD:AddCmdCall(self,self.DoLogin,"l","login game")
	CMD:AddCmdCall(self,self.DoConnectServer,"conn","connect game")
	CMD:AddCmdCall(self,self.DoEnterRoom,"room","enter room")

end

function LoginModule:getUid()
    return self._account
end

function LoginModule:getCid()
    return self._cid
end

function LoginModule:getSid()
    return ACCOUNT_SID[self._account] or 1
end

function LoginModule:genToken()
    return "testToken1"
end

function LoginModule:DoConnectServer()
    local ip = SERVER_IP
    local port = SERVER_PORT

    if GAME_ADDR[GAME_INDEX] then
        ip = GAME_ADDR[GAME_INDEX].ip
        port = GAME_ADDR[GAME_INDEX].port
    end
	print("Connect ",ip," ",port)
    self._M.NetModule:Connect(ip,port,EventID.ON_CONNECT_LOGIN_SERVER)
end

function LoginModule:OnConnect(sock)
    self._Net:SetGameSock(sock)

    if GameType == GAME_TYPE.SEND_PACK then
        return
    end

    print("connect sock:",sock)
    local logpb = {
		account = self:getUid(),
	}
	
	-- logpb.token = string.rep("1234567891",400)
	self._Net:SendGameData("ReqLogin",CMCODE.CM_LOGIN,logpb)

	-- self._Net:SendGameData("ClientMsgLogin",3888,logpb)

	-- self._M.Schedule:AddIntervalTask(function ()
	-- 	self._Net:SendGameData("ClientMsgLogin",3888,logpb)
	-- end,50)
end

function LoginModule:OnConnectRoom(sock)
	print("connect room success ",sock)
	self._Net:SetGameSock(sock)

	local pb = {
		pid = self._room.pid
	}
	self._Net:SendGameData("ReqEnterRoom",CMCODE.CM_ENTER_ROOM,pb)
end

function LoginModule:DoLogin()
    if self._role then
        print("allready login")
        return
    end
    local logpb = {
		account = self:getUid(),
	}

	self._Net:SendGameData("ReqLogin",CMCODE.CM_LOGIN,logpb)
end

function LoginModule:OnTest(data)

	print("get test data",NOW_TICK)
	self._Net:SendGameData("ClientMsgLogin",3888,data)
end

function LoginModule:onLogin(data)

	print("login -------- ")
	printTable(data)
	self._room = data
end

function LoginModule:onGetRole(data)
	print("get role list")
	printTable(data)
end

function LoginModule:OnCharactorList(data)
    print("get charactor list")
    print(persent.block(data))

    self._sid = data.sid
    self._role = data.chrs

    if #data.chrs == 0 then
        -- self:CreateRole()
    else
        self:DoEnterGame({1})
    end
end

function LoginModule:OnCreateReply( data )
    print("OnCreateReply ---- ")
    print(persent.block(data))
end

function LoginModule:DoCreateRole(pam)
    if self._role and #self._role > 3 then
        print("allready have role")
        return
    end
    self:CreateRole(pam[1])
end

function LoginModule:CreateRole(_name)
    local pb = {
        name = _name or self._account,
		pid = self._room.pid
    }
    self._Net:SendGameData("ReqCreateRole",CMCODE.CM_CREATE_ROLE,pb)
end

function LoginModule:DoEnterGame(pam)
	local index = tonumber(pam[1]) or 1
    self:EnterGame(self._role[index])
end

function LoginModule:DoEnterRoom()
	

	print("Connect room ",self._room.ip," ",self._room.port)
    self._M.NetModule:Connect(self._room.ip,self._room.port,EventID.ON_CONNECT_ROOM_SERVER)
end

function LoginModule:EnterGame(role)
    local pb = {
        uid = self:getUid(),
        sid = self:getSid(),
        cid = role.cid,
    }
    self._Net:SendGameData("ClientMsgEnterGame",CMCODE.CM_ENTER_GAME,pb)
end

return LoginModule