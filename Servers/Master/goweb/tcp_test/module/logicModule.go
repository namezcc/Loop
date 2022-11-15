package module

import (
	"fmt"
	"tcp_test/handle"
	"tcp_test/network"
	"tcp_test/util"
	"time"
)

type logicModule struct {
	modulebase
	_server_connid int
	_net           *netModule
}

// 5s 一次
var connectTime int64 = 5000
var ServerAddr = "127.0.0.1:11001"

func (m *logicModule) Init(mgr *moduleMgr) {
	m._mod_mgr = mgr
	m._server_connid = network.CONN_STATE_DISCONNECT
	util.EventMgr.AddEventCall(util.EV_CONN_CLOSE, m, m.onServerClose)

	handle.Handlemsg.AddMsgCall(handle.N_WBE_TEST2, m.onTest2)
	handle.Handlemsg.AddMsgCall(handle.M_SERVER_CONNECTED, m.onServerConnect)
}

func (m *logicModule) AfterInit() {
	m._net = ModuleMgr.GetModule(MOD_NET).(*netModule)
	// m._net.ConnectServer(ServerAddr, &m.modulebase, m.onReadConnBuff)
}

func (m *logicModule) onServerClose(d interface{}) {
	cid := d.(int)
	if cid == m._server_connid {
		m._server_connid = network.CONN_STATE_DISCONNECT
	}
}

func (m *logicModule) onServerConnect(msg handle.BaseMsg) {
	cid := msg.Data.(int)
	if cid != network.CONN_STATE_DISCONNECT {
		m._server_connid = cid
		pack := network.NewMsgPack(128)
		pack.WriteInt32(111)
		m._net.SendPackMsg(m._server_connid, handle.N_WBE_TEST, pack)
		fmt.Println("connect server", ServerAddr)
	} else {
		m.setAfterFunc(time.Second*5, func(int64) {
			m._net.ConnectServer(ServerAddr, &m.modulebase, m.onReadConnBuff)
		})
	}
}

func (m *logicModule) onTest2(msg handle.BaseMsg) {
	p := msg.Data.(network.Msgpack)
	v1 := p.ReadInt32()
	v2 := p.ReadInt64()
	v3 := p.ReadString()
	fmt.Println(v1, v2, string(v3))
}
