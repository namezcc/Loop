LOG_LEVEL = {
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    DEBUG = 4,
}

orgprint = print
print = function( ... )
    CLuaState.callPrint(LOG_LEVEL.INFO,...)
end

-- log_info = print

log_warn = function( ... )
    CLuaState.callPrint(LOG_LEVEL.WARN,...)
end

log_error = function( ... )
    CLuaState.callPrint(LOG_LEVEL.ERROR,...)
end

log_debug = function( ... )
    CLuaState.callPrint(LOG_LEVEL.DEBUG,...)
end

function printTable( tab )
    print(persent.block(tab))
end