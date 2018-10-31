function class(classname, super)  
    local superType = type(super)  
    local cls  
  
    if superType ~= "function" and superType ~= "table" then  
        superType = nil  
        super = nil  
    end  
  
    if superType == "function" or (super and super.__ctype == 1) then  
        -- inherited from native C++ Object  
        cls = {}  
  
        if superType == "table" then  
            -- copy fields from super  
            for k,v in pairs(super) do cls[k] = v end  
            cls.__create = super.__create  
            cls.super    = super  
        else  
            cls.__create = super  
            cls.ctor = function() end  
        end  
  
        cls.__cname = classname  
        cls.__ctype = 1  
  
        function cls.new(...)  
            local instance = cls.__create(...)  
            -- copy fields from class to native object  
            for k,v in pairs(cls) do instance[k] = v end  
            instance.class = cls  
            instance:ctor(...)  
            return instance  
        end  
  
    else  
        -- inherited from Lua Object  
        if super then  
            cls = {}  
            setmetatable(cls, {__index = super})  
            cls.super = super  
        else  
            cls = {ctor = function() end}  
        end  
  
        cls.__cname = classname  
        cls.__ctype = 2 -- lua  
        cls.__index = cls  
  
        function cls.new(...)  
            local instance = setmetatable({}, cls)  
            instance.class = cls  
            instance:ctor(...)  
            return instance  
        end  
    end  
  
    return cls  
end

table.getn = function(tab)
    if tab == nil then
        return 0
    end
    local num = 0
    for k,v in pairs(tab) do
        num = num + 1
    end
    return num
end


function table.clone(tab)
    if tab == nil or type(tab) ~= "table" then
        return nil
    end
    local res = {}
    for k,v in pairs(tab) do
        if type(k) == "table" then
            if type(v) == "table" then
                res[table.clone(k)] = table.clone(v)
            else
                res[table.clone(k)] = v
            end
        else
            if type(v) == "table" then
                res[k] = table.clone(v)
            else
                res[k] = v
            end
        end
    end
    return res
end