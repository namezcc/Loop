package module

import (
	"fmt"
	"net/http"
	"sync"
	"tcp_test/handle"
	"tcp_test/network"
	"tcp_test/util"

	"github.com/gin-gonic/gin"
)

type msgResponse struct {
	_c       chan *network.Msgpack
	_endtime int64
	_id      int
}

type httpModule struct {
	modulebase
	_msg_response   map[int]msgResponse
	_rsp_index      int
	_rsp_index_lock sync.Mutex
	_checkRespTick  int64
	_log_mod        *logicModule
	_net_mod        *netModule
}

func (m *httpModule) Init(mgr *moduleMgr) {
	m._mod_mgr = mgr
	m._msg_response = make(map[int]msgResponse)
	m._rsp_index = 0
	m._checkRespTick = 0
	m._log_mod = mgr.GetModule(MOD_LOGIC).(*logicModule)
	m._net_mod = mgr.GetModule(MOD_NET).(*netModule)

	handle.Handlemsg.AddMsgCall(handle.N_WBE_ON_RESPONSE, m.onMsgRespones)
}

func (m *httpModule) AfterInit() {

	router := gin.Default()

	router.GET("/", m.apiIndex)

	// http.HandleFunc("/", func(rw http.ResponseWriter, r *http.Request) {

	// 	fmt.Println(r.Method, r.URL)

	// 	mc := m.requestMsg(handle.N_WBE_REQUEST_1)
	// 	if mc == nil {
	// 		fmt.Println("requestMsg get nil chan")
	// 		return
	// 	}
	// 	pack := <-mc
	// 	if pack == nil {
	// 		rw.Write([]byte("getpack nil"))
	// 		return
	// 	}

	// 	rw.Write(pack.ReadString())
	// })
	go func() {
		router.Run(":8999")
		// http.ListenAndServe("127.0.0.1:8999", nil)
		fmt.Println("start http server")
	}()
}

func (m *httpModule) Update(dt int64) {
	m.checkRespones(dt)
}

func (m *httpModule) requestMsg(mid int) chan *network.Msgpack {
	if m._log_mod._server_connid == network.CONN_STATE_DISCONNECT {
		return nil
	}

	mrsp := m.getRespones()
	pack := network.NewMsgPack(32)
	pack.WriteInt32(int32(mrsp._id))
	m._net_mod.SendPackMsg(m._log_mod._server_connid, mid, pack)
	return mrsp._c
}

func (m *httpModule) getRespones() msgResponse {
	m._rsp_index_lock.Lock()
	defer m._rsp_index_lock.Unlock()
	msg := msgResponse{
		_c:       make(chan *network.Msgpack),
		_endtime: util.GetMillisecond() + 500000, //5s
		_id:      m._rsp_index,
	}
	m._msg_response[m._rsp_index] = msg
	m._rsp_index++
	return msg
}

func (m *httpModule) checkRespones(dt int64) {
	if dt < m._checkRespTick {
		return
	}

	m._checkRespTick = dt + 5000
	m._rsp_index_lock.Lock()
	defer m._rsp_index_lock.Unlock()

	for k, v := range m._msg_response {
		if dt > v._endtime {
			v._c <- nil
			delete(m._msg_response, k)
		}
	}
}

func (m *httpModule) onMsgRespones(pack network.Msgpack) {
	index := pack.ReadInt32()
	m._rsp_index_lock.Lock()
	rsp, ok := m._msg_response[int(index)]
	if !ok {
		m._rsp_index_lock.Unlock()
		return
	}
	delete(m._msg_response, int(index))
	m._rsp_index_lock.Unlock()

	rsp._c <- &pack
}

func (m *httpModule) apiIndex(c *gin.Context) {
	fmt.Println(c.Request.Method, c.Request.URL)

	mc := m.requestMsg(handle.N_WBE_REQUEST_1)
	if mc == nil {
		fmt.Println("requestMsg get nil chan")
		return
	}
	pack := <-mc
	if pack == nil {
		c.String(http.StatusOK, "getpack nil")
		return
	}

	c.String(http.StatusOK, string(pack.ReadString()))
}
