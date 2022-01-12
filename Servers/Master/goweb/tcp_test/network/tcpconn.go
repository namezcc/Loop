package network

import (
	"fmt"
	"io"
	"net"
)

const (
	CONN_STATE_DISCONNECT = -1
)

type ReadCallBack func(int, int, []byte)
type onClose func(int)

type Tcpconn struct {
	_conn     net.Conn
	_sendchan chan []byte
	_connId   int
	_close    bool
	_onClose  onClose
}

func newTcpconn(id int, conn net.Conn, _c onClose) *Tcpconn {
	_tc := new(Tcpconn)
	_tc._conn = conn
	_tc._connId = id
	_tc._sendchan = make(chan []byte, 100)
	_tc._close = false
	_tc._onClose = _c

	go func() {
		for s := range _tc._sendchan {
			_, err := conn.Write(s)
			if err != nil {
				break
			}
		}

		if !_tc._close {
			_tc._onClose(_tc._connId)
		}
	}()

	return _tc
}

func (_tc *Tcpconn) StartRead(_call ReadCallBack) {
	go func() {
		head := make([]byte, 12)
		for {
			_, e := io.ReadFull(_tc._conn, head)
			if e != nil {
				fmt.Println(e)
				break
			}

			pack := Msgpack{}
			pack.Init(0, 0, head)
			_s := pack.ReadInt32() - HEAD_SIZE
			mid := pack.ReadInt32()

			if _s <= 0 || _s > 655350 {
				fmt.Println("read size too big", _s)
				break
			}

			buff := make([]byte, _s)
			_, e2 := io.ReadFull(_tc._conn, buff)
			if e2 != nil {
				fmt.Println(e2)
				break
			}
			_call(_tc._connId, int(mid), buff)
		}

		if !_tc._close {
			_tc._onClose(_tc._connId)
		}
	}()
}

func (_tc *Tcpconn) Write(b []byte) {
	_tc._sendchan <- b
}

func (_tc *Tcpconn) Close() {
	_tc._close = true
	_tc._conn.Close()
	close(_tc._sendchan)
}
