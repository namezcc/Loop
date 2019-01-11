_Math = {}

function _Math.Normalise(x,y)
    local len = math.sqrt(x*x+y*y)
    return x/len,y/len
end

function ten_to_x(n,x)
    if n < 0 or x < 0 then
        error("error param -num")
    end
    local res = {}
    while(true)
    do
        if n < x then
            table.insert(res,n)
            break
        end

        local yu = n % x
        table.insert(res,yu)
        n = math.floor(n / x)
    end
    return res
end

function clone_tab_simple(tab)
    local res = {}
    for k,v in pairs(tab) do
        res[k] = v
    end
    return res
end

function reverse_tab(tab)
    tab = clone_tab_simple(tab)
    local l = #tab
    local b = 1
    while(b < l)
    do
        tab[b],tab[l] = tab[l],tab[b]
        b = b + 1
        l = l - 1
    end
    return tab
end

function x_divise_n(tab,x,n)
    local l = #tab
    local res = {}
    local divnum = 0
    for i=1,l do
        divnum = divnum*x + tab[i]
        if divnum >= n then
            local xn = math.floor(divnum/n)
            divnum = divnum % n
            table.insert(res,xn)
        elseif #res > 0 then
            table.insert(res,0)
        end
    end
    return res
end

function x_mult_n(tab,x,n)
    tab = reverse_tab(tab)
    local l = #tab
    local res = {}
    local addon = 0
    for i=1,l do
        addon = addon + tab[i]*n
        local xn = addon % x
        table.insert(res,xn)
        addon = math.floor(addon / x)
    end
    while(addon > 0)
    do
        local xn = addon % x
        table.insert(res,xn)
        addon = math.floor(addon / x)
    end
    return reverse_tab(res)
end

function x_plus(tab1,tab2,x)
    tab1 = reverse_tab(tab1)
    tab2 = reverse_tab(tab2)

    if #tab2 > #tab1 then
        tab1,tab2 = tab2,tab1
    end
    local l = #tab1

    for i=1,l do
        tab1[i] = tab1[i] + (tab2[i] or 0)
    end

    local addon = 0
    for i=1,l do
        tab1[i] = tab1[i] + addon
        local xn = tab1[i] % x
        addon = math.floor(tab1[i]/x)
        tab1[i] = xn
    end
    while(addon > 0)
    do
        local xn = addon % x
        addon = math.floor(addon/x)
        table.insert(tab1,xn)
    end
    return reverse_tab(tab1)
end