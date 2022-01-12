package module

import (
	"fmt"
	"net"
	"tcp_test/handle"
	"tcp_test/network"
	"tcp_test/util"

	"github.com/golang/protobuf/proto"
)

type connServer struct {
	mod  *modulebase
	addr string
	call network.ReadCallBack
}

type msgConn struct {
	c    net.Conn
	call network.ReadCallBack
}

type msgSend struct {
	cid  int
	pack network.Msgpack
}

type netModule struct {
	modulebase
	_connMgr *network.ConnMgr
}

func (m *netModule) Init(mgr *moduleMgr) {
	m._mod_mgr = mgr
	m._connMgr = network.NewconnMgr()

	handle.Handlemsg.AddMsgCall(handle.M_CONNECT_SERVER, m.onConnectServer)
	handle.Handlemsg.AddMsgCall(handle.M_ACCEPT_CONN, m.onAcceptConn)
	handle.Handlemsg.AddMsgCall(handle.M_SEND_MSG, m.onSendMsg)
	handle.Handlemsg.AddMsgCall(handle.M_CONN_CLOSE, m.onDoConnClose)
}

func (m *netModule) AfterInit() {

}

func (m *netModule) ConnectServer(addr string, mod *modulebase, readcall network.ReadCallBack) {
	d := connServer{
		addr: addr,
		mod:  mod,
		call: readcall,
	}
	m.sendMsg(handle.M_CONNECT_SERVER, d)
}

func (m *netModule) AcceptConn(c net.Conn, readcall network.ReadCallBack) {
	d := msgConn{
		c:    c,
		call: readcall,
	}
	m.sendMsg(handle.M_ACCEPT_CONN, d)
}

func (m *netModule) SendMsg(cid int, mid int, pb proto.Message) {
	buf, e := proto.Marshal(pb)
	if e != nil {
		fmt.Println(e)
		return
	}
	sendbuf := make([]byte, len(buf)+network.HEAD_SIZE)
	pack := network.Msgpack{}
	pack.Init(cid, mid, sendbuf)
	pack.EncodeMsg(mid, buf)
	m.sendMsg(handle.M_SEND_MSG, pack)
}

func (m *netModule) SendPackMsg(cid int, mid int, pack network.Msgpack) {
	sendbuf := make([]byte, pack.Size()+network.HEAD_SIZE)
	sendpack := network.Msgpack{}
	sendpack.Init(cid, mid, sendbuf)
	sendpack.EncodeMsg(mid, pack.GetBuff())
	m.sendMsg(handle.M_SEND_MSG, sendpack)
}

func (m *netModule) onConnectServer(msg handle.BaseMsg) {
	d := msg.Data.(connServer)

	cid := m._connMgr.ConnectTo(d.addr, d.call, m.onConnClose)
	d.mod.sendMsg(handle.M_SERVER_CONNECTED, cid)
}

func (m *netModule) onAcceptConn(msg handle.BaseMsg) {
	d := msg.Data.(msgConn)
	m._connMgr.AddConn(d.c, d.call, m.onConnClose)
}

func (m *netModule) onSendMsg(msg handle.BaseMsg) {
	d := msg.Data.(msgSend)
	m._connMgr.SendMsg(d.cid, d.pack.GetBuff())
}

func (m *netModule) onConnClose(cid int) {
	m.sendMsg(handle.M_CONN_CLOSE, cid)
}

func (m *netModule) onDoConnClose(msg handle.BaseMsg) {
	cid := msg.Data.(int)

	m._connMgr.CloseConn(cid)
	util.EventMgr.CallEvent(util.EV_CONN_CLOSE, cid)
}
