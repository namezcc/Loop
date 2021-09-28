package handle

import "tcp_test/network"

const (
	MS_BEGIN              = 2000
	N_WBE_TEST            = 2001
	N_WBE_TEST2           = 2002
	N_WBE_ON_RESPONSE     = 2003
	N_WBE_REQUEST_1       = 2004
	N_WEB_VIEW_MACHINE    = 2005
	N_WEB_GET_SERVER_INFO = 2006
	N_WEB_SERVER_OPT      = 2007
	MS_END                = 3000
)

type handlemsg struct {
	_handle []network.HandleFunc
}

var Handlemsg = handlemsg{}

func (_h *handlemsg) InitMsg() {
	_h._handle = make([]network.HandleFunc, MS_END-MS_BEGIN)

}

func (_h *handlemsg) AddMsgCall(mid int, _f network.HandleFunc) {
	_h._handle[mid-MS_BEGIN] = _f
}

func (_h *handlemsg) CallHandle(msg network.Msgpack) {
	mid := msg.MsgId()

	if _h._handle[mid-MS_BEGIN] != nil {
		_h._handle[mid-MS_BEGIN](msg)
	}
}
