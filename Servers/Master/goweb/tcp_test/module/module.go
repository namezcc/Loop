package module

import (
	"tcp_test/handle"
	"tcp_test/interface_func"
	"tcp_test/network"
	"time"
)

type module interface {
	interface_func.Imodule
	Init(*moduleMgr)
	AfterInit()
	Run()
}

type timeCall struct {
	d    int64
	call func(int64)
}

type eventCallData struct {
	d    interface{}
	call interface_func.EventCall
}

type modulebase struct {
	_mod_mgr  *moduleMgr
	_msg_chan chan *handle.BaseMsg
}

var defalut_msg_size int = 100

func (m *modulebase) initMsgSize(s int) {
	m._msg_chan = make(chan *handle.BaseMsg, s)

	handle.Handlemsg.AddMsgCall(handle.M_TIME_CALL, m.onTimeCall)
	handle.Handlemsg.AddMsgCall(handle.M_EVENT_CALL, m.onEventCall)
}

func (m *modulebase) SendEvent(f interface_func.EventCall, d interface{}) {
	m.sendMsg(handle.M_EVENT_CALL, eventCallData{
		d:    d,
		call: f,
	})
}

func (m *modulebase) sendMsg(mid int, d interface{}) {
	m._msg_chan <- &handle.BaseMsg{
		Mid:  mid,
		Data: d,
	}
}

func (m *modulebase) onReadConnBuff(cid int, mid int, buf []byte) {
	pack := network.Msgpack{}
	pack.Init(cid, mid, buf)
	m.sendMsg(mid, pack)
}

func (m *modulebase) onTimeCall(msg handle.BaseMsg) {
	d := msg.Data.(timeCall)
	d.call(d.d)
}

func (m *modulebase) onEventCall(msg handle.BaseMsg) {
	d := msg.Data.(eventCallData)
	d.call(d.d)
}

func (m *modulebase) setAfterFunc(d time.Duration, call func(int64)) {
	go func() {
		c := time.After(d)
		dt := <-c
		msg := timeCall{
			d:    dt.UnixNano() / 1e6,
			call: call,
		}
		m.sendMsg(handle.M_TIME_CALL, msg)
	}()
}

func (m *modulebase) setTikerFunc(d time.Duration, call func(int64)) {
	go func() {
		for v := range time.Tick(d) {
			msg := timeCall{
				d:    v.UnixNano() / 1e6,
				call: call,
			}
			m.sendMsg(handle.M_TIME_CALL, msg)
		}
	}()
}

func (m *modulebase) Run() {
	m.initMsgSize(defalut_msg_size)
	go func() {
		for msg := range m._msg_chan {
			handle.Handlemsg.CallHandle(*msg)
		}
	}()
}
