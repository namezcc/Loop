local NetModule = class()
local SockMap = {}
local SOCK_INDEX = 0

LoopState:callFunction("BindTcpCall","OnRead",function(sock, ... )
    local netmod = SockMap[sock]
    if netmod then
        netmod:OnSocketRead(sock,...)
    end
end)
LoopState:callFunction("BindTcpCall","OnClose",function(sock, ... )
    local netmod = SockMap[sock]
    if netmod then
        netmod:OnSocketClose(sock,...)
    end
end)

function NetModule:init(mods)
    self._conn = {}
    self._handle = {}
    self._Event = mods.Event
    self._PM = mods.SendPackModule
end

function NetModule:Connect(ip,port,eid)
    local res = LoopState:callFunction("AddTcpConnect",SOCK_INDEX+1,ip,port)
    if res == false then
        return 0
    else
        SOCK_INDEX = SOCK_INDEX + 1
        SockMap[SOCK_INDEX] = self
        self._conn[SOCK_INDEX] = true
        self._Event:DoEvent(eid,SOCK_INDEX)
    end
end

function NetModule:SetGameSock( sock )
    self._gameSock = sock
end

function NetModule:SendData(sock,pbStruct,mid,data)
    local s = TPB.encode("LPMsg."..pbStruct,data)
    LoopState:callFunction("SendStreamData",sock,mid,s)
end

function NetModule:SendGameData( pbStruct,mid,data )
    self:SendData(self._gameSock,pbStruct,mid,data)
end

function NetModule:OnSocketRead(sock,mid,data)
    if GameType == GAME_TYPE.SEND_PACK then
        print("recv msgId ------ > ",mid)
        -- print("msgdata -----> ",data)
        self._PM:OnPack(mid,data)
        return
	end
	
	if SHOW_MID == true then
		print("recv msgId ------ > ",mid)
	end
	MID_MAP[mid] = true

    local handle = self._handle[mid]
    if handle then
        if handle.pbstr then
            data = TPB.decode("LPMsg."..handle.pbstr,data)
        end
        if handle.caller then
            handle.func(handle.caller,data,handle.extra)
        else
            handle.func(data,handle.extra)
        end
    end
end

function NetModule:OnSocketClose(sock)
    self._Event:DoEvent(EventID.ON_SERVER_CONN_CLOSE,sock)
    print("sock close ",sock)
end

function NetModule:AddMsgCallBack(mid,_func,_caller,_pbstr,_extra)
    if GameType == GAME_TYPE.SEND_PACK then
        return
    end

    self._handle[mid] = {
        func = _func,
        caller = _caller,
		pbstr = _pbstr,
		extra = _extra,
    }
end

return NetModule