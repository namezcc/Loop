Event = {
	_events = {}
}

function Event:init()
    
end

function Event:AddEvent(evId,_func)
    local etab = self._events[evId]
    if etab == nil then
        etab = {}
        self._events[evId] = etab
    end
    table.insert( etab,{
        func = _func,
    })
end

function Event:DoEvent(caller,evId ,...)
    local etab = self._events[evId]
    if etab then
        for i,v in ipairs(etab) do
			v.func(caller,...)
        end
    end
end
