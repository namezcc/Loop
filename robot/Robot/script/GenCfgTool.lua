-- Cfg -> lua 工具

local InputPath = "../../../../bin/cfgdata/"
local OutPath = "./cfgdata/"

local files = {
	"map",
	"attr",
}

local function Split(szFullString, szSeparator)  
	local nFindStartIndex = 1
	local nSplitIndex = 1
	local nSplitArray = {}
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

TypeFunc["i"] = function(f,s)
	return f._name.."="..s
end

TypeFunc["c"] = function(f,s)
	return f._name.."='"..s.."'"
end

function WriteHead( whand )
	whand:write("local data = {\n")
end

function ParseField( str ,index)
	local vec = Split(str,"\t")
	if #vec < 4 then
		return nil
	end

	local ft = string.match( vec[4],"%[R:(%w*)%]")
	if ft == nil then
		return
	end

	local field = {}
	field._name = vec[1]
	field._type = ft
	field._index = index
	return field
end

function WriteData( whand ,field,data)
	data = Split(data,"\t")
	if #data < #field then
		return
	end
	local ds = ""
	for i,v in ipairs(field) do
		ds = ds..TypeFunc[v._type](v,data[v._index])..","
	end
	ds = string.sub(ds, 1, -2)
	local str = string.format("\t[%s] = {%s},\n",data[1],ds)
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

function CfgToLua(fname,tname)
    local handle = io.open(fname,'r')
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
					local field = ParseField(line,findex)
					findex = findex + 1
					if field then
						table.insert( fields,field )
					end
				end
			elseif ptype == PTYPE.DATA then
				WriteData(whandle,fields,line)
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
	CfgToLua(InputPath..v..".cfg",OutPath..v..".lua")
end

-- local str = ' 12 mname { a= 1,b ="ccc",d = 5.0 }   '
-- --(%{.+%})
-- local i,n,s = string.match( str, "([0-9]+)%s+([%w_]+)%s+({.+})")
-- print(i,n,s)
