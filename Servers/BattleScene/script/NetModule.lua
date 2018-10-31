require "MsgDefine"

NetModule = {
    _handle = {}
}

LoopState:callFunction("BindLuaNetCall",function(...)
    NetModule:OnReadMsg(...)
end)

function NetModule:AddMsgCallBack(mid,_func,_caller,pbStruct)
    self._handle[mid] = {
        func = _func,
        caller = _caller,
        pbstr = pbStruct,
    }
end

function NetModule:OnReadMsg(mid,data)
    local handle = self._handle[mid]
    if handle then
        if handle.pbstr then
            data = pbparse:DecodeToObject(handle.pbstr,data)
        end
        if handle.caller then
            handle.func(handle.caller,data)
        else
            handle.func(data)
        end
    end
end

function NetModule:SendData(pbStruct,mid,data)
    local s = pbparse:EncodeToStream(pbStruct,data)
    LoopState:callFunction("SendStreamData",mid,s)
    pbparse:clearStream(s)
end