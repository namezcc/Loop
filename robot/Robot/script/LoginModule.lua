local LoginModule = {}

local _Net = NetModule

function LoginModule:init()

    Event:AddEvent(EventID.ON_CONNECT_LOGIN_SERVER,self.OnConnect)
	Event:AddEvent(EventID.ON_CONNECT_ROOM_SERVER,self.OnConnectRoom)

	_Net:AddMsgCallBack(SMCODE.SM_LOGIN_RES,self.onLogin,"RoomInfo")
	_Net:AddMsgCallBack(SMCODE.SM_SELF_ROLE_INFO,self.onGetRole,"SmRoleList")
	-- self._Net:AddMsgCallBack(SMCODE.SM_CLIENT_REPLY,self.OnCreateReply,self,"ServerMsgClientReply")
	-- self._Net:AddMsgCallBack(3888,self.OnTest,self,"ClientMsgLogin")

    CMD:AddCmdCall(self.DoCreateRole,"crole","pam:name : create role")
    CMD:AddCmdCall(self.DoEnterGame,"enter","pam:roleindex :enter game")
    CMD:AddCmdCall(self.DoLogin,"l","login game")
	CMD:AddCmdCall(self.DoConnectServer,"c","connect game")
	CMD:AddCmdCall(self.DoEnterRoom,"room","enter room")
	CMD:AddCmdCall(self.DoCloseConnect,"close","close connect")
end

function LoginModule:onNewPlayer(account)
	self._account = account

	if AUTO_ENTER then
		Schedule:AddIntervalTask(function()
			self:DoConnectServer()
		end,0,1,1)
	end

end

function LoginModule:SendGameData(pbname,mid,data)
	_Net:SendData(self._sock,pbname,mid,data)
end

function LoginModule:getUid()
    return self._account
end

function LoginModule:getCid()
    return self._cid
end

function LoginModule:genToken()
    return "testToken1"
end

function LoginModule:DoConnectServer()
    local ip = SERVER_IP
    local port = SERVER_PORT

    if IS_SINGLE and GAME_ADDR[GAME_INDEX] then
        ip = GAME_ADDR[GAME_INDEX].ip
        port = GAME_ADDR[GAME_INDEX].port
    end
	if IS_SINGLE then
		print("Connect ",ip," ",port)
	end
    local res = _Net:Connect(ip,port,EventID.ON_CONNECT_LOGIN_SERVER,self)
	if res == false then
		if AUTO_ENTER then
			print("connect fail try after 3s...")
		end
		if AUTO_RECONNECT then
			Schedule:AddIntervalTask(function()
				self:DoConnectServer()
			end,0,3000,1)
		end
	end
end

function LoginModule:OnConnect(sock)
    self._sock = sock

    if GameType == GAME_TYPE.SEND_PACK then
        return
    end

	if IS_SINGLE then
		print("connect sock:",sock)
	end
    local logpb = {
		account = self:getUid(),
	}
	
	-- logpb.token = string.rep("1234567891",400)
	self:SendGameData("ReqLogin",CMCODE.CM_LOGIN,logpb)

end

function LoginModule:OnConnectRoom(sock)
	if IS_SINGLE then
		print("connect room success ",sock)
	end
	self._sock = sock

	Schedule:AddIntervalTask(function()
		local pb = {
			uid = self._room.uid,
			key = self._room.key
		}
		self:SendGameData("CmEnterGame",CMCODE.CM_ENTER_ROOM,pb)
	end,0,500,1)
end

function LoginModule:DoLogin()
    if self._role then
        print("allready login")
        return
    end
    local logpb = {
		account = self:getUid(),
	}

	self:SendGameData("ReqLogin",CMCODE.CM_LOGIN,logpb)
end

function LoginModule:OnTest(data)

	print("get test data",NOW_TICK)
	self:SendGameData("ClientMsgLogin",3888,data)
end

function LoginModule:onLogin(data)

	if IS_SINGLE then
		print("login -------- ")
		printTable(data)
	end
	self._room = data
	if AUTO_ENTER then
		self:DoEnterRoom()
	end
end

function LoginModule:onGetRole(data)
	if IS_SINGLE then
		print("get role list")
		printTable(data)
	end
	self._role = data.roles

	if AUTO_ENTER then
		if #self._role > 0 then
			self:EnterGame(self._role[1])
		else
			self:DoCreateRole({self._account.."-r1"})
		end
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
		cid = self._room.uid,
    }
    self:SendGameData("ReqCreateRole",CMCODE.CM_CREATE_ROLE,pb)
end

function LoginModule:DoEnterGame(pam)
	local index = tonumber(pam[1]) or 1
    self:EnterGame(self._role[index])
end

function LoginModule:DoEnterRoom()
	
	if IS_SINGLE then
		print("Connect room ",self._room.ip," ",self._room.port)
	end
    _Net:Connect(self._room.ip,self._room.port,EventID.ON_CONNECT_ROOM_SERVER,self)
end

function LoginModule:DoCloseConnect()
	_Net:CloseSock(self._sock)
end

function LoginModule:EnterGame(role)
    local pb = {
        value = role.cid,
    }
    self:SendGameData("Int32Value",CMCODE.CM_ENTER_GAME,pb)
	print(role.name," enter room ------->> ",self._room.ip,"   ",NOW_TICK - BEG_TIME," ms")
end

return LoginModule