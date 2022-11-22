require "split"
local Lex = require "lex"
local lua_path = "../../_out/Debug/lua/room/util"

local fname = {
	"../../proto/client/define.proto",
	"../../proto/server/server_msgid.proto"
}

function find_message(l,getmsg,res)
	while not l:eof() do
		local kw = l:keyword()
		if kw == "enum" then
			local ms = l:keyword()
			local str = l:match_pat("{.-}")
			if getmsg[ms] then
				res[getmsg[ms]] = str
			end
		elseif kw == "message" then
			l:keyword()
			l:match_pat("{.-}")
		else
			l:line_end()
		end
	end
end

local getmsg = {
	LP_CM_MSG_ID = "CM",
	LP_SM_MSG_ID = "SM",
	IM_MSG_ID = "SERVER_MSG",
}

function parse_proto(ftab)
	local plex = Lex.new()
	local res = {}

	for i, f in ipairs(ftab) do
		plex:read_file(f)
		find_message(plex,getmsg,res)
	end

	return res
end

local res = parse_proto(fname)
local genorder = {"CM","SM","SERVER_MSG"}

local file_lua = myfile.new()
file_lua:open(lua_path.."/msg_enum.lua","w")
file_lua:write("--文件是自动生成, tool/make_msg_enum.bat\n")

for i, k in ipairs(genorder) do
	local v = res[k]

	file_lua:write(k.." = ")

	v = string.gsub(v,";",",")
	v = string.gsub(v,"//","--")
	file_lua:write(v)
	file_lua:write("\n")
end