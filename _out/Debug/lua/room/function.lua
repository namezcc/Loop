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

local IMPL_FUNC = {}
local INIT_DB_FUNC = {}
local LOGOUT_FUNC = {}

IMPL_FUNC["initDB"] = function(ply,...)
	player_init_db(ply,...)
end

IMPL_FUNC["logout"] = function(ply)
	for i, f in ipairs(LOGOUT_FUNC) do
		f(ply)
	end
end

local function imp_func( ply,fname )
	return IMPL_FUNC[fname]
end

-- 绑定模块函数到玩家的成员函数,不能有相同的函数名会覆盖,init除外
function bind_player_func( mod )
	for k,v in pairs(mod) do
		if type(v) == "function" and k ~= "init" and k ~= "initDB" and k ~= "logout" then
			if IMPL_FUNC[k] ~= nil then
				LOG_ERROR("have same name %s !!! need change",k)
			end
			IMPL_FUNC[k] = v
		elseif k == "initDB" then
			INIT_DB_FUNC[#INIT_DB_FUNC+1] = v
		elseif k == "logout" then
			LOGOUT_FUNC[#LOGOUT_FUNC+1] = v
        end
    end
end

function player_init_db(ply,data)
	for index, f in ipairs(INIT_DB_FUNC) do
		f(ply,data)
	end
end

function setPlayerImpFunc( ply )
	setmetatable(ply, {__index = imp_func})
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

table.getOrNewTable = function( tab,key )
	local t = tab[key]
	if t == nil then
		t = {}
		tab[key] = t
	end
	return t
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

LFUNC = {
	CallCFunc = CallCFunc,
	player_init_db = player_init_db,
}