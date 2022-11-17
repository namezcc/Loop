ROOM_MGR_SERVER = {
	type = SERVER_TYPE.ST_ROOM_MANAGER,
	id = 1,
}

SELF_SERVER = {
	type = SERVER_TYPE.ST_ROOM,
	id = 0,
}

function set_self_server_id(id)
	SELF_SERVER.id = id
end
