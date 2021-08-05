local Event = class()

function Event:ctor()
    self._events = {}
end

function Event:init()
    
end

function Event:AddEvent(evId,_func,_caller)
    local etab = self._events[evId]
    if etab == nil then
        etab = {}
        self._events[evId] = etab
    end
    table.insert( etab,{
        func = _func,
        caller = _caller
    })
end

function Event:DoEvent(evId ,...)
    local etab = self._events[evId]
    if etab then
        for i,v in ipairs(etab) do
            if v.caller then
                v.func(v.caller,...)
            else
                v.func(...)
            end
        end
    end
end

return Event