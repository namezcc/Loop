local SendPackModule = {}

function SendPackModule:init()
	
end

-------------------- cmds----------

function StrToTable(str)
    if str == nil or type(str) ~= "string" then
        return
    end
    
    return loadstring("return " .. str)()
end

function SendPackModule:SendPack( cmdstr )

    local ml = string.match( cmdstr, "([%w]+)")

    if ml == "send" then
        local _1,mids,mname,ds = string.match( cmdstr, "([%w]+)%s+([0-9]+)%s+([%w_]+)%s+({.+})")
        local mid = tonumber(mids)
        local data = StrToTable(ds)
        -- printTable(data)
        self:SendGameData(mname,mid,data)
    elseif ml == "read" then
        local _1,mids,mname = string.match( cmdstr, "([%w]+)%s+([0-9]+)%s+([%w_]+)")
        self:ParsePack(tonumber(mids),mname)
    else
        print("error cmd",ml)
    end
end

function SendPackModule:ParsePack( mid,name )
    local data = self._pack[mid]
    if data == nil then
        print("no pack")
        return
    end

    local obj = TPB.decode("LPMsg."..name,data)
    print("data ----------- :")
    printTable(obj)
end

function SendPackModule:OnPack(mid,data)
    self._pack[mid] = data
end

return SendPackModule