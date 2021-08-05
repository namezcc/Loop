-- Cfg -> lua 工具

local InputPath = "../cfgdata/"
local OutPath = "./cfgdata/"
local tinsert = table.insert
local files = {
	"gem",
	"item",
	"level",
	"npc",
	"tianfuPoint",
	"subTianfu",

	"task",
	"equip",
	"equip_affix",
	"equip_affix_series",
	"equip_affix_group",
	"equip_affix_grade",
	"equip_legend",
	"equip_suit",
	"formula",
	"yaoji",
	"soul",
	"resolve",
	"effect",
	"master",
	"formula",
	"rune_suit",
	"rune_suit_attr",
	"rune_suit_restore",
	"map_trap",
	"npc",
	"npc_shop",
	"static_param",
	"dungeon_event",
	"drop_group",
	"task_zhixian_ratio",
	"compound_level",
	--"task_event",
	--"task_event_process",
	--"task_puzzle",
}

-- 数组方式导出
local arr = {
	["yaoji"] = true,
	["soul"] = true,
}

local P_Int = 1
local P_Vec = 2
local P_String = 3
local P_VecTable = 4
local P_ComplexTable = 5
local P_Table = 6
local P_VecVec = 7

local ToFunc = {
	["i"] = P_Int,
	["s"] = P_String,
	["v"] = P_Vec,
	["t"] = P_Table,
	["vt"] = P_VecTable,
	["ct"] = P_ComplexTable,
	["vv"] = P_VecVec,
}

local function Split(szFullString, szSeparator)  
	local nFindStartIndex = 1
	local nSplitIndex = 1
	local nSplitArray = {}
	if szFullString == "" then
		return nSplitArray
	end

	while true do
		local nFindLastIndex = string.find(szFullString, szSeparator, nFindStartIndex)
		if not nFindLastIndex then
			nSplitArray[nSplitIndex] = string.sub(szFullString, nFindStartIndex, string.len(szFullString))
			break
		end
		nSplitArray[nSplitIndex] = string.sub(szFullString, nFindStartIndex, nFindLastIndex - 1)
		nFindStartIndex = nFindLastIndex + string.len(szSeparator)
		nSplitIndex = nSplitIndex + 1
	end
	return nSplitArray
end

local TypeFunc = {}

TypeFunc[P_Int] = function(f,s)
	return f._name.."="..(s == '' and 0 or s)
end

TypeFunc[P_String] = function(f,s)
	return f._name.."='"..s.."'"
end

TypeFunc[P_Vec] = function(f,s)
	s = string.gsub(s,":",",")
	return f._name.."={"..s.."}"
end

TypeFunc[P_Table] = function(f,s)
	local vec = Split(s,"|")
	local res = {}
	for i,v in ipairs(vec) do
		local p = Split(v,":")
		res[i] = string.format( "[%s]=%s",p[1],p[2] )
	end
	local ns = table.concat(res, ",")
	return f._name.."={"..ns.."}"
end

TypeFunc[P_VecTable] = function(f,s)
	local vec = Split(s,":")

	for i,v in ipairs(vec) do
		vec[i] = string.format( "[%s]=true",v )
	end

	local ns = table.concat( vec, ",")
	return f._name.."={"..ns.."}"
end

TypeFunc[P_VecVec] = function(f,s)
	local vec = Split(s,"|")
	for i,v in ipairs(vec) do
		vec[i] = "{"..string.gsub( v,":",",").."}"
	end
	local ns = table.concat( vec, ",")
	return f._name.."={"..ns.."}"
end

TypeFunc[P_ComplexTable] = function(f, s)
	if s == "0" then
		return f._name .. "={}"
	end

	local ret_array = {}
	local vertical_array = Split(s, "|")
	for _, v in ipairs(vertical_array) do
		local tmp_tab = {}
		local colon_array = Split(v, ":")
		for _, inner_v in ipairs(colon_array) do
			tinsert(tmp_tab, inner_v)
		end

		--tinsert(ret_array, "{")
		tinsert(ret_array, "{" .. table.concat(tmp_tab, ",") .. "}")
		--tinsert(ret_array, "}")
	end
	return f._name .. "={" .. table.concat(ret_array, ",") .. "}"
end

function getParseFunc( str )
	local s = string.match(str,"%[sl:(%w+)%]")
	return ToFunc[s]
end

function WriteHead( whand )
	whand:write("local data = {\n")
end

function ParseField(fname, str ,index)
	local vec = Split(str,"\t")
	if #vec < 4 then
		return
	end
	
	local ft = getParseFunc(vec[4])
	if ft == nil then
		return
	end
	-- local ft = P_String;
	-- if outConf[fname] then-- 要导出的字段
	-- 	ft = outConf[fname][vec[1]]
	-- 	if ft == nil then
	-- 		return
	-- 	end
	-- else --全字段导出
	-- 	if (vec[2] == "int") then
	-- 		ft = P_Int;
	-- 	end
	-- end
	local field = {}
	field._name = vec[1]
	field._type = ft
	field._index = index
	return field
end

function WriteData( whand ,field,data,fname)
	data = string.gsub( data,"\r","" )
	data = Split(data,"\t")
	if #data < #field then
		return
	end
	local ds = ""
	for i,v in ipairs(field) do
		ds = ds..TypeFunc[v._type](v,data[v._index])..","
	end
	ds = string.sub(ds, 1, -2)
	local str = nil
	if arr[fname] then
		str = string.format("\t{%s},\n",ds)
	else
		str = string.format("\t[%s] = {%s},\n",data[1],ds)
	end
	whand:write(str)
end

function WriteEnd(whand)
	whand:write("}\n\nreturn data\n")
end

PTYPE = {
	NONE = 0,
	HEAD = 1,
	DATA = 2,
}

function CfgToLua(fname, tname, confName)
    local handle = io.open(fname, 'r')
	if handle == nil then
		print("error no file",fname)
		return
	end
	local whandle = io.open(tname,'w')
	if whandle == nil then
		print("error open write file ",tname)
		return
	end

	local line = nil
	local ptype = PTYPE.NONE
	local fields = {}
	local findex = 1
	
	WriteHead(whandle)

	repeat
		line = handle:read("*l")
		if line then
			if ptype == PTYPE.HEAD then
				if string.find( line,"data" ) then
					ptype = PTYPE.DATA
				else
					local field = ParseField(confName,line,findex)
					findex = findex + 1
					if field then
						tinsert( fields,field )
					end
				end
			elseif ptype == PTYPE.DATA then
				WriteData(whandle,fields,line,confName)
			elseif ptype == PTYPE.NONE then
				if string.find( line,"header") then
					ptype = PTYPE.HEAD
				end
			end
		end
	until(line == nil)

	WriteEnd(whandle)

	handle:close()
	whandle:close()
	print("success :",tname)
end

for i,v in ipairs(files) do
	CfgToLua(InputPath..v..".cfg",OutPath..v..".lua",v)
end

-- local str = ' 12 mname { a= 1,b ="ccc",d = 5.0 }   '
-- --(%{.+%})
-- local i,n,s = string.match( str, "([0-9]+)%s+([%w_]+)%s+({.+})")
-- print(i,n,s)