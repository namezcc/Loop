package network

import "encoding/binary"

var _order = binary.LittleEndian

type Msgpack struct {
	_buff      []byte
	_index     int
	_max       int
	_connId    int
	_msgId     int
	_connBroad []int
}

func NewMsgPack(size int) Msgpack {
	pack := Msgpack{
		_buff:  make([]byte, size),
		_index: 0,
		_max:   size,
	}
	return pack
}

func NewMsgPackDef() Msgpack {
	pack := Msgpack{
		_buff:  make([]byte, 64),
		_index: 0,
		_max:   64,
	}
	return pack
}

func (_m *Msgpack) Init(cid int, mid int, b []byte) {
	_m._buff = b
	_m._index = 0
	_m._max = cap(b)
	_m._connId = cid
	_m._msgId = mid
}

func (_m *Msgpack) SetBroad(cids []int) {
	_m._connBroad = append(_m._connBroad, cids...)
}

func (_m *Msgpack) BroadCid() []int {
	return _m._connBroad
}

func (_m *Msgpack) ReadInt8() int8 {
	if _m._index+1 > _m._max {
		return 0
	}
	_m._index += 1
	return int8(_m._buff[_m._index-1])
}

func (_m *Msgpack) ReadInt16() int16 {
	if _m._index+2 > _m._max {
		return 0
	}
	_m._index += 2
	return int16(_order.Uint16(_m._buff[_m._index-2 : _m._index]))
}

func (_m *Msgpack) ReadInt32() int32 {
	if _m._index+4 > _m._max {
		return 0
	}
	_m._index += 4
	return int32(_order.Uint32(_m._buff[_m._index-4 : _m._index]))
}

func (_m *Msgpack) ReadInt64() int64 {
	if _m._index+8 > _m._max {
		return 0
	}
	_m._index += 8
	return int64(_order.Uint64(_m._buff[_m._index-8 : _m._index]))
}

func (_m *Msgpack) ReadString() []byte {
	_s := _m.ReadInt32()
	_m._index += int(_s)
	return _m._buff[_m._index-int(_s) : _m._index]
}

func (_m *Msgpack) ReadBuff() []byte {
	return _m._buff[_m._index:_m._max]
}

func (_m *Msgpack) expandSize(s int) {
	if _m._max+s > _m._max*2 {
		_m._max = _m._max + s
	} else {
		_m._max = _m._max * 2
	}
	tmp := make([]byte, _m._max)
	copy(tmp, _m._buff)
	_m._buff = tmp
}

func (_m *Msgpack) WriteInt8(v int8) {
	if _m._index+1 > _m._max {
		_m.expandSize(1)
	}
	_m._buff[_m._index] = byte(v)
	_m._index += 1
}

func (_m *Msgpack) WriteInt16(v int16) {
	if _m._index+2 > _m._max {
		_m.expandSize(2)
	}
	_order.PutUint16(_m._buff[_m._index:_m._index+2], uint16(v))
	_m._index += 2
}

func (_m *Msgpack) WriteInt32(v int) {
	if _m._index+4 > _m._max {
		_m.expandSize(4)
	}
	_order.PutUint32(_m._buff[_m._index:_m._index+4], uint32(v))
	_m._index += 4
}

func (_m *Msgpack) WriteInt64(v int64) {
	if _m._index+8 > _m._max {
		_m.expandSize(8)
	}
	_order.PutUint64(_m._buff[_m._index:_m._index+8], uint64(v))
	_m._index += 8
}

func (_m *Msgpack) WriteString(v []byte) {
	_s := len(v)
	_m.WriteInt32(_s)
	if _m._index+_s > _m._max {
		_m.expandSize(_s)
	}
	copy(_m._buff[_m._index:_m._index+_s], v)
	_m._index += _s
}

func (_m *Msgpack) WriteRealString(v string) {
	_s := len(v)
	_m.WriteInt32(_s)
	if _m._index+_s > _m._max {
		_m.expandSize(_s)
	}
	copy(_m._buff[_m._index:_m._index+_s], v)
	_m._index += _s
}

func (m *Msgpack) WriteBuff(v []byte) {
	if m._index+len(v) > m._max {
		m.expandSize(len(v))
	}
	copy(m._buff[m._index:m._index+len(v)], v)
	m._index += len(v)
}

func (_m *Msgpack) ConnId() int {
	return _m._connId
}

func (_m *Msgpack) MsgId() int {
	return _m._msgId
}

func (_m *Msgpack) GetBuff() []byte {
	return _m._buff[:_m._index]
}

const HEAD_SIZE = 12

func (_m *Msgpack) EncodeMsg(mid int, buf []byte) {
	_m.WriteInt32(HEAD_SIZE + len(buf))
	_m.WriteInt32(mid)
	_m.WriteInt32(0)
	if len(buf) > 0 {
		_m.WriteBuff(buf)
	}
}

func (m *Msgpack) Size() int {
	return m._index
}
