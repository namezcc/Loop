package module

import (
	"net"
	"tcp_test/util"
)

type ServerModule struct {
	modulebase
	_net *netModule
}

func (m *ServerModule) Init(mgr *moduleMgr) {
	m._mod_mgr = mgr
}

func (m *ServerModule) AfterInit() {
	m._net = ModuleMgr.GetModule(MOD_NET).(*netModule)

	m.startListen()
}

func (m *ServerModule) startListen() {
	go func() {
		host := util.GetConfValue("host")

		ln, err := net.Listen("tcp", host)

		if err != nil {
			println(err)
			return
		} else {
			println("start listen", host)
		}

		for {
			conn, err := ln.Accept()
			if err != nil {
				println(err)
			} else {
				println(util.NowTime(), "accept conn", conn.RemoteAddr().String())
				m._net.AcceptConn(conn, m.onReadConnBuff)
			}
		}
	}()
}
