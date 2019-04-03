
require "Math"
local persent = require "persent"
math.randomseed(tostring(os.time()):reverse():sub(1, 6))

CODE_LEN = 6
GEN_COUNT = 2000
CODE_MAP = '0123456789'     --ABCDEFGHIJKLMNOPQRSTUVWXYZ
CODE_FILE = 'code.txt'

local tmp = {}
for w in string.gmatch(CODE_MAP, "%w") do
    tmp[#tmp+1] = w
end
CODE_MAP = tmp
CODE_MAP_LEN = #tmp

CODE_MAX_NUM = {}

for i=1,CODE_LEN do
    CODE_MAX_NUM[i] = CODE_MAP_LEN - 1
end

STATIC_CODE = {
    --[1] = '1'
}

function printTab(tab,desc)
    if desc then
        print(desc)
    end
    print(persent.block(tab))
end

function randomTab(tab,n)
    n = n or 100
    local l = #tab
    for i=1,n do
        local x = math.random(2,l)
        tab[1],tab[x] = tab[x],tab[1]
    end
    return tab
end

local codecash = {}
function checkSame(code)
    if codecash[code] then
        print("error gen same code:",code)
        return true
    else
        codecash[code] = true
    end
end

CODE_SOURCE_LIST = {}
for i=1,CODE_LEN do
    CODE_SOURCE_LIST[i] = randomTab(clone_tab_simple(CODE_MAP))
end

CODE_ADDON = x_divise_n(CODE_MAX_NUM,CODE_MAP_LEN,GEN_COUNT)
if #CODE_ADDON == 0 then
    CODE_ADDON[1] = 1
end

CODE_ADDON_SUB = x_divise_n(CODE_ADDON,CODE_MAP_LEN,CODE_MAP_LEN)

-- printTab(CODE_SOURCE_LIST,"CODE_SOURCE_LIST")
-- printTab(CODE_ADDON,"CODE_ADDON")
-- printTab(CODE_ADDON_SUB,"CODE_ADDON_SUB")

outfile = io.open(CODE_FILE,"w")
local codetab = {}

function genCode(n)
    local baseIndex = x_mult_n(CODE_ADDON,CODE_MAP_LEN,n-1)
    local mult = math.random(0,CODE_MAP_LEN-2)
    local randadd = x_mult_n(CODE_ADDON_SUB,CODE_MAP_LEN,mult)

    local resIndex = x_plus(baseIndex,randadd,CODE_MAP_LEN)

    resIndex = reverse_tab(resIndex)

    for i=1,CODE_LEN do
        if STATIC_CODE[i] then
            codetab[i] = STATIC_CODE[i]
        else
            local index = (resIndex[i] or 0) + 1
            codetab[i] = CODE_SOURCE_LIST[i][index]
        end
    end

    local codestr = table.concat( codetab, "")
    -- local resstr = table.concat(resIndex," ")

    -- checkSame(codestr)

    outfile:write(codestr)
    outfile:write('\n')
    -- print(codestr)
end

-- local n = 0

-- for i=1,GEN_COUNT do
--     genCode(i)
-- end

-- outfile:close()
-- print("gen down")

local numTab = {}
local nline = 16
for i=1,256/nline do
    numTab[i] = {}
end

local begidx = 0
local begcmp = 1

for i=0,255 do
    local idx = math.floor(i/nline) + 1
    if i >= begcmp then
        begidx = begidx + 1
        begcmp = begcmp*2
    end
    table.insert(numTab[idx],begidx-1)
end

for i,v in ipairs(numTab) do
    print(table.concat( v, ","))
end