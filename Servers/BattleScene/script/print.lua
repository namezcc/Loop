function PrintTable(tab,lv)
    if type(tab) ~= "table" then
        return
    end
    local elv = lv or 1
    local tap = '  '
    tap = string.rep(tap,elv)
    for k,v in pairs(tab) do
        local str = ""
        if type(k) == "number" then
            str = tostring(k)
        elseif type(k) == "string" then
            str = k
        elseif type(k) == "function" then
            str = "function"
        elseif type(k) == "table" then
            str = "table"
        else
            str = "unkone key type"
        end

        if type(v) == "number" then
            str = str..":"..tostring(v)
        elseif type(v) == "string" then
            str = str..":"..v
        elseif type(v) == "function" then
            str = str.."function : "..tostring(v)
        elseif type(v) == "table" then
            if k == "class" or k == "_scene" then
                print(tap..str.." = {}")
            else
                print(tap..str.." = ")
                PrintTable(v,elv+1)
            end
        end
        if type(v) ~= "table" then
            print(tap..str)
        end
    end
end