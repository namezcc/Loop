package module

import "tcp_test/handle"

const (
	MOD_LOGIC = iota
	MOD_NET
	MOD_HTTP
	MOD_END
)

type moduleMgr struct {
	_mod []module
}

var ModuleMgr moduleMgr

func (m *moduleMgr) Init() {
	m._mod = make([]module, MOD_END-MOD_LOGIC)
	handle.Handlemsg.InitMsg()

	m._mod[MOD_LOGIC] = &logicModule{}
	m._mod[MOD_NET] = &netModule{}
	m._mod[MOD_HTTP] = &httpModule{}

	for _, v := range m._mod {
		v.Init(m)
	}

	for _, v := range m._mod {
		v.AfterInit()
	}
}

func (m *moduleMgr) Update(dt int64) {
	for _, v := range m._mod {
		v.Update(dt)
	}
}

func (m *moduleMgr) GetModule(mid int) module {
	return m._mod[mid]
}
