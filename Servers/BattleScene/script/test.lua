--print("test")
math.randomseed(tostring(os.time()):reverse():sub(1, 7))

function max_heap_insert(_arr,n)
    local max_num = #_arr + 1
    _arr[max_num] = n
    max_heap_filter_up(_arr,max_num)
end

function max_heap_filter_up(_arr,index)
    while(true)
    do
        local f = math.ceil((index-1)/2)
        if f <= 0 then
            return
        end
        local fn = _arr[f]
        local cn = _arr[index]
        if cn > fn then
            _arr[f],_arr[index] = _arr[index],_arr[f]
            index = f
        else
            break
        end
    end
end

function max_heap_filter_down(_arr,index)
    local num = #_arr
    while(true)
    do
        local l = 2*index
        if l > num then
            return
        end
        if l + 1 <= num then
            if _arr[l] <_arr[l+1] then
                l = l+1
            end
        end
        local cn = _arr[index]
        if cn > _arr[l] then
            return
        else
            _arr[l],_arr[index] = _arr[index],_arr[l]
            index = l
        end
    end
end

function max_heap_filter(_arr,index)
    local num = #_arr
    if index == 1 then
        max_heap_filter_down(_arr,index)
    elseif index >= num then
        max_heap_filter_up(_arr,num)
    else
        local fi = math.ceil((index-1)/2)
        if _arr[fi] < _arr[index] then
            max_heap_filter_up(_arr,index)
        else
            max_heap_filter_down(_arr,index)
        end
    end
end

function max_heap_delete(_arr,index)
    local num = #_arr
    if index < 1 or index > num then
        return
    end
    _arr[index] = _arr[num]
    _arr[num] = nil
    max_heap_filter(_arr,index)
end

function max_heap_change(_arr,index,n)
    local num = #_arr
    if index < 1 or index > num then
        return
    end
    _arr[index] = n
    max_heap_filter(_arr,index)
end

function randHeap(_arr,num,min,max)
    for i=1,num do
        max_heap_insert(_arr,math.random(min,max))
    end
end

function show_heap(_arr)
    print("-----------------")
    local lv = 1
    local num = #_arr
    local n = 0
    while(true)
    do
        if n > num then
            return
        end
        local str = ""
        for i=1,lv do
            if n + i > num then
                break
            end
            local nv = _arr[n+i]
            str = str.."  "..tostring(nv)
        end
        print(str)
        n = n + lv
        lv = lv*2
    end
end

function check_max_heap(_arr)
    local num = #_arr
    for i=1,num do
        local l = 2*i
        local r = 2*i+1
        if l <= num then
            if _arr[i] < _arr[l] then
                print("error max heap:",i,l,_arr[i],_arr[l])
                return false
            end
        end
        if r <= num then
            if _arr[i] < _arr[r] then
                print("error max heap:",i,r,_arr[i],_arr[r])
                return false
            end
        end
    end
    return true
end

function Test_max_heap_add(_arr,min,max)
    max_heap_insert(_arr,math.random(min,max))
    -- if not check_max_heap(_arr) then
    --     print("Test_max_heap_add")
    -- end
end

function Test_max_heap_del(_arr,index)
    max_heap_delete(_arr,index)
    -- if not check_max_heap(_arr) then
    --     print("Test_max_heap_del")
    -- end
end

function Test_max_heap_chg(_arr,index,min,max)
    max_heap_change(_arr,index,math.random(min,max))
    -- if not check_max_heap(_arr) then
    --     print("Test_max_heap_chg")
    -- end
end

local OPT_TYPE = {
    INSERT = 1,
    DELETE = 2,
    CHANGE = 3,
}

function Test_max_heap_opt(num,opts)
    for i=1,num do
        local tmparr = {}
        for j,v in ipairs(opts) do
            if v.opt == OPT_TYPE.INSERT then
                for k=1,v.num do
                    Test_max_heap_add(tmparr,v.min,v.max)
                end
            elseif v.opt == OPT_TYPE.DELETE then
                for k=1,v.num do
                    local index = math.random(1,#tmparr)
                    Test_max_heap_del(tmparr,index)
                end
            else
                for k=1,v.num do
                    local index = math.random(1,#tmparr)
                    Test_max_heap_chg(tmparr,index,v.min,v.max)
                end
            end
        end
    end
end


-- local arr = {}

-- randHeap(arr,30,1,10000)
-- show_heap(arr)
-- check_max_heap(arr)

-- max_heap_delete(arr,2)
-- show_heap(arr)
-- check_max_heap(arr)

-- Test_max_heap_chg(arr,10,1,10000)
-- show_heap(arr)
-- check_max_heap(arr)

local optt = {
    {
        opt = OPT_TYPE.INSERT,
        num = 1000,
        min = 1,
        max = 1000000,
    },
    {
        opt = OPT_TYPE.CHANGE,
        num = 1000,
        min = 1,
        max = 1000000,
    },
    {
        opt = OPT_TYPE.DELETE,
        num = 100,
        min = 1,
        max = 1000000,
    },
    {
        opt = OPT_TYPE.CHANGE,
        num = 1000,
        min = 1,
        max = 1000000,
    },
    {
        opt = OPT_TYPE.DELETE,
        num = 100,
        min = 1,
        max = 1000000,
    },
    {
        opt = OPT_TYPE.INSERT,
        num = 500,
        min = 1,
        max = 1000000,
    },
}

Test_max_heap_opt(10,optt)