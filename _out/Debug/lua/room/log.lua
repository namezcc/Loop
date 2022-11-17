local cstate = CLuaState
local LOG_LEVEL = {
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
}

orgprint = print
local LOG_DEBUG = function(fmt,p1,... )
    local msg = fmt
    if p1 ~= nil then
       msg = string.format(fmt,p1, ...)
    end
    cstate.callPrint(LOG_LEVEL.DEBUG, msg)
end

local LOG_INFO = function(fmt,p1,... )
    local msg = fmt
    if p1 ~= nil then
       msg = string.format(fmt,p1, ...)
    end
    cstate.callPrint(LOG_LEVEL.INFO,msg)
end


local LOG_WARN = function( fmt,p1,... )
    local msg = fmt
    if p1 ~= nil then
       msg = string.format(fmt,p1, ...)
    end
    cstate.callPrint(LOG_LEVEL.WARN, msg)
end


LOG_ERROR = function(fmt,p1,... )
    local msg = fmt
    if p1 ~= nil then
       msg = string.format(fmt,p1, ...)
    end
    cstate.callPrint(LOG_LEVEL.ERROR,msg)
end

function printTable(tab)
    print(persent.block(tab))
end

function printTableKey( tab )
	for k,v in pairs(tab) do
		print(k)
	end
end

print = LOG_INFO

LOG = {
    debug = LOG_DEBUG,
    info = LOG_INFO,
    warn = LOG_WARN,
    error = LOG_ERROR,
}
