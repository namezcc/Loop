package network

import (
	"container/list"
	"fmt"
	"net"
	"sync"
	"tcp_test/util"
)

type HandleFunc func(Msgpack)

type connMgr struct {
	_connvec     []*Tcpconn
	_connId_pool []int
	_pool_index  int
	_close_list  list.List
	_close_lock  sync.Mutex
	_msg_list    list.List
	_msg_lock    sync.Mutex
	_handfunc    HandleFunc
}

type ConnMgr struct {
	connMgr
}

func NewconnMgr(handle HandleFunc) *ConnMgr {
	mgr := ConnMgr{}

	mgr._pool_index = 0
	mgr._handfunc = handle
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

func (_m *connMgr) AddConn(c net.Conn) int {
	cid := _m.popConnId()
	conn := newTcpconn(cid, c, _m.onConnClose)
	conn.StartRead(_m.onReadMsg)
	_m._connvec[cid] = conn
	return cid
}

func (_m *connMgr) onConnClose(cid int) {
	_m._close_lock.Lock()
	_m._close_list.PushBack(cid)
	_m._close_lock.Unlock()
}

func (_m *connMgr) Update() {
	_m.doCloseConn()
	_m.handleMsg()
}

func (_m *connMgr) doCloseConn() {
	_m._close_lock.Lock()
	if _m._close_list.Len() == 0 {
		_m._close_lock.Unlock()
		return
	}
	el := _m._close_list.Front()
	_m._close_list.Init()
	_m._close_lock.Unlock()

	for el != nil {
		cid := el.Value.(int)
		_m.closeConn(cid)
		util.EventMgr.CallEvent(util.EV_CONN_CLOSE, cid)
		el = el.Next()
	}
}

func (_m *connMgr) closeConn(cid int) {
	conn := _m._connvec[cid]
	if conn == nil {
		fmt.Println("error close conn nil")
		return
	}

	_m._connvec[cid] = nil
	_m.pushConnId(cid)
	conn.Close()
}

func (_m *connMgr) onReadMsg(cid int, mid int, buf []byte) {
	pack := Msgpack{}
	pack.Init(cid, mid, buf)
	_m._msg_lock.Lock()
	_m._msg_list.PushBack(pack)
	_m._msg_lock.Unlock()
}

func (_m *connMgr) handleMsg() {
	_m._msg_lock.Lock()
	if _m._msg_list.Len() == 0 {
		_m._msg_lock.Unlock()
		return
	}

	el := _m._msg_list.Front()
	_m._msg_list.Init()
	_m._msg_lock.Unlock()

	for el != nil {
		pack := el.Value.(Msgpack)
		_m._handfunc(pack)
		el = el.Next()
	}
}

func (_m *connMgr) ConnectTo(addr string) int {
	conn, e := net.Dial("tcp", addr)
	if e != nil {
		fmt.Println(e)
		return -1
	}

	return _m.AddConn(conn)
}

func (_m *connMgr) SendMsg(cid int, buf []byte) {
	conn := _m._connvec[cid]
	if conn == nil {
		return
	}
	conn.Write(buf)
}

func (_m *connMgr) CloseConn(cid int) {
	_m.closeConn(cid)
}
