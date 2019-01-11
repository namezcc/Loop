local BN = 0
local function N(n)
    if n then
        BN = n
        return n
    end
    BN = BN + 1
    return BN 
end

PROP_TYPE = {
    PT_HP = N(),
    PT_POS_X = N(),
    PT_POS_Y = N(),
    PT_V_X = N(),
    PT_V_Y = N(),
}