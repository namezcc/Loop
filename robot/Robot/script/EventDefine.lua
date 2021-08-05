local n = 0
local function N(_n)
    if _n then
        n = _n
    else
        n = n+1
    end
    return n
end

EventID = {
    ON_GAME_INIT = N(1),
    ON_CONNECT_LOGIN_SERVER = N(),
    ON_CONNECT_ROOM_SERVER = N(),
    ON_SERVER_CONN_CLOSE = N(),
    ON_UDP_CONNECT = N(),
    ON_UDP_CLOSE = N(),
}

