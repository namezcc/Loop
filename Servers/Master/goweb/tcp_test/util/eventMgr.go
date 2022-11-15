package util

import (
	"container/list"
	"tcp_test/interface_func"
)

type callData struct {
	mod  interface_func.Imodule
	call interface_func.EventCall
}

type eventMgr struct {
	_callMap map[int]*list.List
}

var EventMgr = eventMgr{
	_callMap: make(map[int]*list.List),
}

func (e *eventMgr) AddEventCall(eid int, mod interface_func.Imodule, call interface_func.EventCall) {
	l, ok := e._callMap[eid]
	if !ok {
		l = list.New()
		e._callMap[eid] = l
	}
	l.PushBack(callData{
		mod:  mod,
		call: call,
	})
}

func (e *eventMgr) CallEvent(eid int, d interface{}) {
	l, ok := e._callMap[eid]
	if !ok {
		return
	}
	for n := l.Front(); n != nil; n = n.Next() {
		v := n.Value.(callData)
		v.mod.SendEvent(v.call, d)
	}
}
