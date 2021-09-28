package module

import (
	"fmt"
	"tcp_test/handle"
	"tcp_test/network"
	"tcp_test/util"
)

type module interface {
	Init(*moduleMgr)
	AfterInit()
	Update(int64)
}

type modulebase struct {
	_mod_mgr *moduleMgr
}

type logicModule struct {
	modulebase
	_server_connid   int
	_checkServerTick int64
	_net             *netModule
}

// 5s 一次
var connectTime int64 = 5000
var ServerAddr = "127.0.0.1:11001"

func (m *logicModule) Init(mgr *moduleMgr) {
	m._mod_mgr = mgr
	m._checkServerTick = 0
	m._server_connid = network.CONN_STATE_DISCONNECT
	util.EventMgr.AddEventCall(util.EV_CONN_CLOSE, m.onServerClose)

	handle.Handlemsg.AddMsgCall(handle.N_WBE_TEST2, m.onTest2)
}

func (m *logicModule) AfterInit() {
	m._net = ModuleMgr.GetModule(MOD_NET).(*netModule)
}

func (m *logicModule) Update(dt int64) {
	m.checkConnectServer(dt)
}

func (m *logicModule) onServerClose(d interface{}) {
	cid := d.(int)
	if cid == m._server_connid {
		m._server_connid = network.CONN_STATE_DISCONNECT
	}
}

func (m *logicModule) checkConnectServer(dt int64) {
	if m._server_connid != network.CONN_STATE_DISCONNECT {
		return
	}
	if dt > m._checkServerTick {
		m._checkServerTick = dt + connectTime
		// if connectTime < 60000 {
		// 	connectTime += 5000
		// }
		cid := m._net.ConnectServer(ServerAddr)
		if cid != network.CONN_STATE_DISCONNECT {
			m._server_connid = cid
			pack := network.NewMsgPack(128)
			pack.WriteInt32(111)
			m._net.SendPackMsg(m._server_connid, handle.N_WBE_TEST, pack)
			fmt.Println("connect server", ServerAddr)
		}
	}
}

func (m *logicModule) onTest2(p network.Msgpack) {
	v1 := p.ReadInt32()
	v2 := p.ReadInt64()
	v3 := p.ReadString()
	fmt.Println(v1, v2, string(v3))
}
