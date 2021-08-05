package module

import (
	"fmt"
	"tcp_test/handle"
	"tcp_test/network"

	"github.com/golang/protobuf/proto"
)

type netModule struct {
	modulebase
	_connMgr *network.ConnMgr
}

func (m *netModule) Init(mgr *moduleMgr) {
	m._mod_mgr = mgr
	m._connMgr = network.NewconnMgr(handle.Handlemsg.CallHandle)
}

func (m *netModule) AfterInit() {

}

func (m *netModule) Update(dt int64) {
	m._connMgr.Update()
}

func (m *netModule) ConnectServer(addr string) int {
	return m._connMgr.ConnectTo(addr)
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
	m._connMgr.SendMsg(cid, pack.GetBuff())
}

func (m *netModule) SendPackMsg(cid int, mid int, pack network.Msgpack) {
	sendbuf := make([]byte, pack.Size()+network.HEAD_SIZE)
	sendpack := network.Msgpack{}
	sendpack.Init(cid, mid, sendbuf)
	sendpack.EncodeMsg(mid, pack.GetBuff())
	m._connMgr.SendMsg(cid, sendpack.GetBuff())
}
