package util

import (
	"container/list"
)

type eventCall func(interface{})

type eventMgr struct {
	_callMap map[int]*list.List
}

var EventMgr = eventMgr{
	_callMap: make(map[int]*list.List),
}

func (e *eventMgr) AddEventCall(eid int, call eventCall) {
	l, ok := e._callMap[eid]
	if !ok {
		l = list.New()
		e._callMap[eid] = l
	}
	l.PushBack(call)
}

func (e *eventMgr) CallEvent(eid int, d interface{}) {
	l, ok := e._callMap[eid]
	if !ok {
		return
	}
	for n := l.Front(); n != nil; n = n.Next() {
		n.Value.(eventCall)(d)
	}
}
