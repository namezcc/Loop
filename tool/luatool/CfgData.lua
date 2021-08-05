_CFG = {}
local CFG = _CFG
local cfgfile = {
    "gem",
	"item",
	"tianfuPoint",
	"subTianfu",
	"yaoji",
	"soul",
	"effect",
}

local function getKeyTabal( d,k )
	local t = d[k]
	if t == nil then
		t = {}
		d[k] = t
	end
	return t
end
-- 生成多级table
local function genNMapFunc( ks )
	return function( data )
		local nt = {}
		for i,v in ipairs(data) do
			local last = nil
			for j=1,#ks-1 do
				last = getKeyTabal(nt,v[ks[j]])
			end
			last[v[ks[#ks]]] = v
		end
		return nt
	end
end

local function genGetFunc2(data)
	return function( k1,k2 )
		return data[k1][k2]
	end
end

-- 表初始化函数
local initfunc = {
	["soul"] = genNMapFunc({"id","level"}),
	["item"] = function( data )
		for k,v in pairs(data) do
			if v.id <= 100 then
				v.effect = tonumber(v.effect)
			end
		end
		return data
	end,
}

-- 获取数据函数
local getfunc = {
	["soul"] = genGetFunc2,
}

local cfgfunc = {}

cfgfunc.table = function( name )
	return CFG[name]
end

local function genGetFunc( name )
	local _tab = CFG[name]

	if getfunc[name] then
		return getfunc[name](_tab)
	else
		return function( id )
			return _tab[id]
		end
	end
end

for i,v in ipairs(cfgfile) do
	local data = require("cfgdata."..v)
	if initfunc[v] then
		data = initfunc[v](data)
	end
    CFG[v] = data
end

for i,v in ipairs(cfgfile) do
    cfgfunc[v] = genGetFunc(v)
end

function ReloadCfgData( orgname )
	local name = "cfgdata."..orgname
	package.loaded[name] = nil
	
	local data = require (name)

	if initfunc[orgname] then
		data = initfunc[orgname](data)
	end

	CFG[orgname] = data
	cfgfunc[orgname] = genGetFunc(name)
end

return cfgfunc