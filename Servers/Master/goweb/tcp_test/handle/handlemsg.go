package handle

const (
	MS_BEGIN = 100

	M_CONNECT_SERVER = MS_BEGIN + iota
	M_SERVER_CONNECTED
	M_ACCEPT_CONN
	M_SEND_MSG
	M_CONN_CLOSE
	M_TIME_CALL
	M_EVENT_CALL
	M_ON_RESPONSE
	M_GET_SERVER_STATE
	M_HOT_LOAD
	M_AI_UPDATE

	N_REGISTE_SERVER      = 303
	N_CONN_SERVER_INFO    = 313
	N_WBE_TEST            = 2001
	N_WBE_TEST2           = 2002
	N_WBE_ON_RESPONSE     = 2003
	N_WBE_REQUEST_1       = 2004
	N_WEB_VIEW_MACHINE    = 2005
	N_WEB_GET_SERVER_INFO = 2006
	N_WEB_SERVER_OPT      = 2007
	N_LOG_ERROR           = 2008
	N_SERVER_REGIST       = 2009
	MS_END                = 3000

	IM_LOGIN_BEGIN       = 12000
	IM_LOGIN_WEB_HOTLOAD = 12500
	IM_LOGIN_AI_UPDATE   = 12501

	IM_LOGIN_END = 13000
)

type BaseMsg struct {
	Mid  int
	Data interface{}
}

type HandleFunc func(BaseMsg)

type handleBlock struct {
	_handle    []HandleFunc
	_msg_begin int
	_msg_end   int
}

type handlemsg struct {
	_block []handleBlock
}

var Handlemsg = handlemsg{}

func (_h *handlemsg) InitMsg(msg_begin int, msg_end int) {
	_h._block = append(_h._block, handleBlock{
		_msg_begin: msg_begin,
		_msg_end:   msg_end,
		_handle:    make([]HandleFunc, msg_end-msg_begin),
	})
}

func (_h *handlemsg) AddMsgCall(mid int, _f HandleFunc) {
	for _, v := range _h._block {
		if v._msg_begin <= mid && v._msg_end >= mid {
			v._handle[mid-v._msg_begin] = _f
			return
		}
	}
}

func (_h *handlemsg) CallHandle(msg BaseMsg) {
	mid := msg.Mid

	for _, v := range _h._block {
		if v._msg_begin <= mid && v._msg_end >= mid {
			if v._handle[mid-v._msg_begin] != nil {
				v._handle[mid-v._msg_begin](msg)
			}
			return
		}
	}
}
