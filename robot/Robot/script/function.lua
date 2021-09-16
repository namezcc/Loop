local IMPL_FUNC = {}

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