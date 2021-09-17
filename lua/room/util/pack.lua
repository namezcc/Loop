local buffpack = class("buffpack")
local _netpack = NetMsg
local PBencode = pb.encode
local PBdecode = pb.decode
local cstate = CLuaState

function buffpack:ctor(len)
	self._pack = _netpack.makepack(cstate,len)
end

function buffpack:buff()
	return self._pack
end

function buffpack:writepb(pb,pbname)
	local buf = PBencode(pbname,pb)
	_netpack.writebuff(self._pack,buf)
end

function buffpack:writeint8(i)
	_netpack.writeint8(self._pack,i)
end

function buffpack:writeint16(i)
	_netpack.writeint16(self._pack,i)
end

function buffpack:writeint32(i)
	_netpack.writeint32(self._pack,i)
end

function buffpack:writeint64(i)
	_netpack.writeint64(self._pack,i)
end

function buffpack:writestring(s)
	_netpack.writestring(self._pack,s)
end

local netpack = class("netpack")

function netpack:ctor(netmsg)
	self._msg = netmsg
end

function netpack:decode(pbname)
	local buf = _netpack.readbuff(self._msg)
	return PBdecode(pbname,buf)
end

function netpack:readint8()
	return _netpack.readint8(self._msg)
end

function netpack:readint16()
	return _netpack.readint16(self._msg)
end

function netpack:readint32()
	return _netpack.readint32(self._msg)
end

function netpack:readint64()
	return _netpack.readint64(self._msg)
end

function netpack:readstring()
	return _netpack.readstring(self._msg)
end

PACK = {
	buffpack = function (len)
		return buffpack.new(len)
	end,

	netpack = function (msg)
		return netpack.new(msg)
	end,

	encodePB = function (pb,proto)
		return PBencode(proto,pb)
	end
}