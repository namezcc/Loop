OPT_BUFF_SIZE = 20
KEEP_MOVE_FRAME_NUM = 1

OPT_TYPE = {
    OPT_TEST = 1,
    OPT_MOVE = 2,
    OPT_STOP = 3,
}

OPT_FUNC = {}
local OF = OPT_FUNC

OF[OPT_TYPE.OPT_TEST] = function(ply,cmd)
    ply:SetProp(PROP_TYPE.PT_HP,cmd.value[1])
end

OF[OPT_TYPE.OPT_MOVE] = function(ply,cmd)
    local nx,ny = _Math.Normalise(cmd.value[1],cmd.value[2])
    ply:SetProp(PROP_TYPE.PT_V_X,math.floor(nx * ply._speed))
    ply:SetProp(PROP_TYPE.PT_V_Y,math.floor(ny * ply._speed))
    ply._moveKeepFrame = KEEP_MOVE_FRAME_NUM
end

OF[OPT_TYPE.OPT_STOP] = function(ply,cmd)
    ply:SetProp(PROP_TYPE.PT_V_X,0)
    ply:SetProp(PROP_TYPE.PT_V_Y,0)
    ply._moveKeepFrame = 0
end