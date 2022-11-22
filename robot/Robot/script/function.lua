local IMPL_FUNC = {}

local initdata = {}

IMPL_FUNC["initdata"] = function (ply)
	for i, f in ipairs(initdata) do
		f(ply)
	end
end

function bind_player_func( mod )
	for k,v in pairs(mod) do
		if type(v) == "function" then
			if k == "initdata" then
				initdata[#initdata+1] = v
			elseif k ~= "init" then
				if IMPL_FUNC[k] ~= nil then
					print("have same name !!! need change ",k)
				end
				IMPL_FUNC[k] = v
			end
        end
    end
end

function bind_player_func( mod )
	for k,v in pairs(mod) do
		if type(v) == "function" and k ~= "init" then
			if IMPL_FUNC[k] ~= nil then
				print("have same name !!! need change ",k)
			end
			IMPL_FUNC[k] = v
        end
    end
end

local function imp_func( ply,fname )
	return IMPL_FUNC[fname]
end

function setPlayerImpFunc( ply )
	setmetatable(ply, {__index = imp_func})
end