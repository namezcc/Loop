local cstate = CLuaState
function CallCFunc( findex,... )
    return cstate:luaCallC(findex,...)
end

function BindLuaFunc(findex, _func )
    cstate:bindLuaFunc(findex, _func)
end

function BindModLuaFunc(findex, mod, _func )
    cstate:bindLuaFunc(findex, function(...)
        return _func(mod,...)
    end)
end

function printCallStack()
	local startLevel = 2 --0表示getinfo本身,1表示调用getinfo的函数(printCallStack),2表示调用printCallStack的函数,可以想象一个getinfo(0级)在顶的栈.
	local maxLevel = 10	--最大递归10层
	local log = {}
	for level = startLevel, maxLevel do
		-- 打印堆栈每一层
		local info = debug.getinfo( level, "nSl") 
		if info == nil then break end
		log[#log+1] = string.format("[ line : %-4d]  %-20s :: %s\n", info.currentline, info.name or "", info.source or "" )

		-- 打印该层的参数与局部变量
		local index = 1 --1表示第一个参数或局部变量, 依次类推
		while true do
			local name, value = debug.getlocal( level, index )
			if name == nil then break end

			local valueType = type( value )
			local valueStr
			if valueType == 'string' then
				if string.len(value) > 500  then
					value = string.sub(value,1,500)
				end
				valueStr = value
			elseif valueType == "number" then
				valueStr = string.format("%.2f", value)
			end
			if valueStr ~= nil then
				log[#log+1] = string.format( "\t%s = %s\n", name, value )
			end
			index = index + 1
		end
	end
	LOG.warn(table.concat(log,""))
end

local IMPL_FUNC = {}
local INIT_DB_FUNC = {}
local AFTER_INIT = {}
local AFTER_ENTER_GAME = {}
local LOGOUT_FUNC = {}
local UPDATE_FUNC = {}

IMPL_FUNC["initDB"] = function(ply,...)
	for k, f in pairs(INIT_DB_FUNC) do
		f(ply,...)
	end
end

IMPL_FUNC["afterInit"] = function(ply,...)
	for k, f in pairs(AFTER_INIT) do
		f(ply,...)
	end
end

IMPL_FUNC["sendClienMsg"] = function(ply,...)
	for k, f in pairs(AFTER_ENTER_GAME) do
		f(ply,...)
	end
end

IMPL_FUNC["logout"] = function(ply)
	for i, f in pairs(LOGOUT_FUNC) do
		f(ply)
	end
end

IMPL_FUNC["update"] = function(ply,...)
	for k, f in pairs(UPDATE_FUNC) do
		f(ply,...)
	end
end

local function imp_func( ply,fname )
	return IMPL_FUNC[fname]
end

-- 绑定模块函数到玩家的成员函数,不能有相同的函数名会覆盖,init除外
function bind_player_func( mod,modname,hotload )
	for k,v in pairs(mod) do
		if type(v) == "function" then
			if k == "initDB" then
				INIT_DB_FUNC[modname] = v
			elseif k == "logout" then
				LOGOUT_FUNC[modname] = v
			elseif k == "afterInit" then
				AFTER_INIT[modname] = v
			elseif k == "sendClienMsg" then
				AFTER_ENTER_GAME[modname] = v
			elseif k == "update" then
				UPDATE_FUNC[modname] = v
			elseif k ~= "init" then
				if hotload == nil and IMPL_FUNC[k] ~= nil then
					LOG_ERROR("have same name %s !!! need change",k)
				end
				IMPL_FUNC[k] = v
			end
        end
    end
end

function setPlayerImpFunc( ply )
	-- setmetatable(ply, {__index = imp_func})
	setmetatable(ply, {__index = IMPL_FUNC})
end

table.copy = function( tab )
    local ntab = {}
    for k,v in pairs(tab) do
        ntab[k] = v
    end
    return ntab
end

table.copyValue = function( dst,src )
    for k,v in pairs(src) do
        dst[k] = v
    end
end

table.insertArray = function( arr,arr2 )
	for i,v in ipairs(arr2) do
		arr[#arr+1] = v
	end
end

table.swapKV = function(tab)
	local ntab = {}
	for k,v in pairs(tab) do
		ntab[v] = k
	end
	return ntab
end

table.empty = function(tab)
    if not tab then
        return true
    end
	return next(tab) == nil
end

table.getn = function(tab)
    if tab == nil then
        return 0
    end
    local num = 0
    for k,v in pairs(tab) do
        num = num + 1
    end
    return num
end

table.getOrNewTable = function( tab,key,val )
	local t = tab[key]
	if t == nil then
		t = vak or {}
		tab[key] = t
	end
	return t
end

function table.cloneSimple(tab)
	local res = {}
	for k, v in pairs(tab) do
		if type(v) == "table" then
			assert(false,"canot clone value type table use this function!!!")
		end
		res[k] = v
	end
	return res
end

function table.clone(tab)
    if tab == nil or type(tab) ~= "table" then
        return nil
    end
    local res = {}
    for k,v in pairs(tab) do
        if type(k) == "table" then
            if type(v) == "table" then
                res[table.clone(k)] = table.clone(v)
            else
                res[table.clone(k)] = v
            end
        else
            if type(v) == "table" then
                res[k] = table.clone(v)
            else
                res[k] = v
            end
        end
    end
    return res
end

local function int32To64(hei,low)
	return (hei << 32)|low
end

local _time = TIME_DATA
local function getNowTick()
	return _time.TICK_TIME_STAMEP
end

local function getNowSecond()
	return _time.TIME_STAMEP
end

local function set_bit_on(v,b)
	return v|(1<<(b-1))
end

local function set_bit_off(v,b)
	return v&(~(1<<(b-1)))
end

local function is_bit_on(v,b)
	return (v&(1<<(b-1)))~=0
end

LFUNC = {
	CallCFunc = CallCFunc,
	int32To64 = int32To64,
	getNowTick = getNowTick,
	getNowSecond = getNowSecond,
	set_bit_on = set_bit_on,
	set_bit_off = set_bit_off,
	is_bit_on = is_bit_on,
}