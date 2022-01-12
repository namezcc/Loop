package network

import (
	"fmt"
	"net"
)

type HandleFunc func(Msgpack)

type connMgr struct {
	_connvec     []*Tcpconn
	_connId_pool []int
	_pool_index  int
}

type ConnMgr struct {
	connMgr
}

func NewconnMgr() *ConnMgr {
	mgr := ConnMgr{}

	mgr._pool_index = 0
	mgr._connvec = make([]*Tcpconn, 1000)
	mgr._connId_pool = make([]int, 1000)

	for i := 0; i < 1000; i++ {
		mgr._connId_pool[i] = i
	}

	return &mgr
}

func (_m *connMgr) popConnId() int {
	_m._pool_index++
	return _m._connId_pool[_m._pool_index-1]
}

func (_m *connMgr) pushConnId(id int) {
	_m._pool_index--
	_m._connId_pool[_m._pool_index] = id
}

func (_m *connMgr) AddConn(c net.Conn, readcall ReadCallBack, closecall onClose) int {
	cid := _m.popConnId()
	conn := newTcpconn(cid, c, closecall)
	conn.StartRead(readcall)
	_m._connvec[cid] = conn
	return cid
}

func (_m *connMgr) CloseConn(cid int) {
	conn := _m._connvec[cid]
	if conn == nil {
		fmt.Println("error close conn nil")
		return
	}

	_m._connvec[cid] = nil
	_m.pushConnId(cid)
	conn.Close()
}

func (_m *connMgr) ConnectTo(addr string, readcall ReadCallBack, closecall onClose) int {
	conn, e := net.Dial("tcp", addr)
	if e != nil {
		fmt.Println(e)
		return CONN_STATE_DISCONNECT
	}

	return _m.AddConn(conn, readcall, closecall)
}

func (_m *connMgr) SendMsg(cid int, buf []byte) {
	conn := _m._connvec[cid]
	if conn == nil {
		return
	}
	conn.Write(buf)
}
