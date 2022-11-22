NetModule = {}
local SockMap = {}
local SOCK_INDEX = 0

LoopState:callFunction("BindTcpCall","OnRead",function(sock, ... )
    NetModule:OnSocketRead(sock,...)
end)
LoopState:callFunction("BindTcpCall","OnClose",function(sock, ... )
	NetModule:OnSocketClose(sock)
end)

function NetModule:init()
    self._conn = {}
    self._handle = {}
end

function NetModule:Connect(ip,port,eid,ply)
    local res = LoopState:callFunction("AddTcpConnect",SOCK_INDEX+1,ip,port)
	
    if res then
        SOCK_INDEX = SOCK_INDEX + 1
        SockMap[SOCK_INDEX] = ply
        self._conn[SOCK_INDEX] = true
        Event:DoEvent(ply,eid,SOCK_INDEX)
    end

	return res
end

function NetModule:SetGameSock( sock )
    self._gameSock = sock
end

function NetModule:SendData(sock,pbStruct,mid,data)
    local s = TPB.encode("LPMsg."..pbStruct,data)
    LoopState:callFunction("SendStreamData",sock,mid,s)
end

function NetModule:OnSocketRead(sock,mid,data)
	local ply = SockMap[sock]
	if ply == nil then
		print("sock ply nil ",sock)
		return
	end

    if GameType == GAME_TYPE.SEND_PACK then
        -- print("recv msgId ------ > ",mid)
        -- print("msgdata -----> ",data)
        ply:OnPack(mid,data)
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

		handle.func(ply,data,handle.extra,handle.pbstr)
    end
end

function NetModule:OnSocketClose(sock)
	local ply = SockMap[sock]
	if ply == nil then
		print("sock close ply nil ",sock)
		return
	end

    Event:DoEvent(ply,EventID.ON_SERVER_CONN_CLOSE,sock)

	if IS_SINGLE then
		print("sock close ",sock)
	end
end

function NetModule:CloseSock(sock)
	LoopState:callFunction("closeConn",sock)
end

function NetModule:AddMsgCallBack(mid,_func,_pbstr,_extra)
    if GameType == GAME_TYPE.SEND_PACK then
        return
    end

    self._handle[mid] = {
        func = _func,
		pbstr = _pbstr,
		extra = _extra,
    }
end