package module

import (
	"context"
	"fmt"
	"tcp_test/handle"
	"tcp_test/network"
	"tcp_test/util"

	"github.com/go-redis/redis/v8"
)

const (
	ST_LOGIN = 2
)

type serverInfo struct {
	stype   int32
	sid     int32
	state   int32
	connid  int
	addrkey string
}

type MasterModule struct {
	ServerModule
	servers      map[int]serverInfo
	_http_mod    *HttpModule
	_rdb         *redis.Client
	_server_conn map[string]int
}

var ctx = context.Background()

func (m *MasterModule) Init(mgr *moduleMgr) {
	m.ServerModule.Init(mgr)

	m.servers = make(map[int]serverInfo)
	m._server_conn = make(map[string]int)

	m._http_mod = mgr.GetModule(MOD_HTTP).(*HttpModule)

	util.EventMgr.AddEventCall(util.EV_CONN_CLOSE, m, m.onServerClose)

	handle.Handlemsg.AddMsgCall(handle.N_REGISTE_SERVER, m.onServerRegist)

	handle.Handlemsg.AddMsgCall(handle.M_GET_SERVER_STATE, m.onGetServerState)
	handle.Handlemsg.AddMsgCall(handle.M_HOT_LOAD, m.onLoginHotLoad)
	handle.Handlemsg.AddMsgCall(handle.M_AI_UPDATE, m.onLoginAiUpdate)

	rhost := util.GetConfValue("redis-host")
	rpass := util.GetConfValue("redis-pass")
	rdb := util.GetConfInt("redis-db")

	m._rdb = redis.NewClient(&redis.Options{
		Addr:     rhost,
		Password: rpass,
		DB:       rdb,
	})

	_, err := m._rdb.Ping(ctx).Result()
	if err != nil {
		panic(err)
	}

	fmt.Printf("redis connect %s\n", rhost)

	// val, err := rdb.Keys(ctx, "key:*").Result()
	// if err != nil {
	// 	fmt.Println(err)
	// }
	// fmt.Println(val)
	// v, e := rdb.MGet(ctx, val...).Result()
	// if e != nil {
	// 	fmt.Println(e)
	// }
	// fmt.Println(v)
}

func (m *MasterModule) onServerRegist(msg handle.BaseMsg) {
	p := msg.Data.(network.Msgpack)
	addrkey := p.ReadString()
	host := p.ReadString()

	var connkey, watchkey, noticekey []string
	num := p.ReadInt8()
	connkey = make([]string, 0, num)
	for i := 0; i < int(num); i++ {
		connkey = append(connkey, string(p.ReadString()))
	}
	num = p.ReadInt8()
	watchkey = make([]string, 0, num)
	for i := 0; i < int(num); i++ {
		watchkey = append(watchkey, string(p.ReadString()))
	}
	num = p.ReadInt8()
	noticekey = make([]string, 0, num)
	for i := 0; i < int(num); i++ {
		noticekey = append(noticekey, string(p.ReadString()))
	}

	_, err := m._rdb.Set(ctx, string(addrkey), host, 0).Result()
	if err != nil {
		fmt.Println(err)
	}
	m._server_conn[string(addrkey)] = p.ConnId()

	connser := make([]string, 0, 10)
	for _, v := range connkey {
		adds, err := m._rdb.Keys(ctx, v).Result()
		if err != nil {
			fmt.Println(err)
			continue
		}
		if len(adds) > 0 {
			res, err := m._rdb.MGet(ctx, adds...).Result()
			if err != nil {
				fmt.Println(err)
				continue
			}
			for _, saddr := range res {
				connser = append(connser, saddr.(string))
			}
		}
	}

	if len(connser) > 0 {
		pack := network.NewMsgPackDef()
		pack.WriteInt32(len(connser))
		for _, v := range connser {
			pack.WriteRealString(v)
		}
		m._net.SendPackMsg(p.ConnId(), handle.N_CONN_SERVER_INFO, pack)
	}

	for _, v := range watchkey {
		_, err := m._rdb.SAdd(ctx, v, addrkey).Result()
		if err != nil {
			fmt.Println(err)
		}
	}

	noticeser := make([]string, 0, 10)
	for _, v := range noticekey {
		res, err := m._rdb.SMembers(ctx, v).Result()
		if err != nil {
			fmt.Println(err)
		} else {
			if len(res) > 0 {
				noticeser = append(noticeser, res...)
			}
		}
	}

	broadcid := make([]int, 0, 10)
	for _, v := range noticeser {
		cid, ok := m._server_conn[v]
		if ok && cid != p.ConnId() {
			broadcid = append(broadcid, cid)
		}
	}

	if len(broadcid) > 0 {
		pack := network.NewMsgPackDef()
		pack.WriteInt32(1)
		pack.WriteRealString(string(host))
		m._net.BroadPackMsg(broadcid, handle.N_CONN_SERVER_INFO, pack)
	}

	info := serverInfo{
		stype:   0,
		sid:     0,
		state:   0,
		connid:  p.ConnId(),
		addrkey: string(addrkey),
	}

	m.servers[info.connid] = info
	fmt.Printf("server reg %s\n", addrkey)
}

func (m *MasterModule) onGetServerState(msg handle.BaseMsg) {
	rep := msg.Data.(responesData)

	serstate := make(map[int32]int32)

	for _, v := range m.servers {
		key := (v.stype << 16) | v.sid
		serstate[key] = v.state
	}

	rep.data = serstate
	m._http_mod.sendMsg(handle.M_ON_RESPONSE, rep)
}

func (m *MasterModule) onServerClose(d interface{}) {
	cid := d.(int)

	info, ok := m.servers[cid]

	if ok {
		delete(m.servers, cid)
		delete(m._server_conn, info.addrkey)
	}
}

func (m *MasterModule) onLoginHotLoad(msg handle.BaseMsg) {
	if len(m.servers) == 0 {
		return
	}

	info := msg.Data.(hotInfo)

	if len(info.Server) > 0 {
		for _, v := range info.Server {
			ser, ok := m.servers[v]
			if ok {
				pack := network.NewMsgPack(len(info.Lua))
				pack.WriteString([]byte(info.Lua))
				m._net.SendPackMsg(ser.connid, handle.IM_LOGIN_WEB_HOTLOAD, pack)
			}
		}
	} else {
		check := make(map[int]int)
		for _, v := range info.Except {
			check[v] = 1
		}

		for _, v := range m.servers {
			if v.stype != ST_LOGIN {
				continue
			}
			_, ok := check[int(v.sid)]
			if ok {
				continue
			}

			pack := network.NewMsgPack(len(info.Lua))
			pack.WriteString([]byte(info.Lua))
			m._net.SendPackMsg(v.connid, handle.IM_LOGIN_WEB_HOTLOAD, pack)
		}
	}
}

func (m *MasterModule) onLoginAiUpdate(msg handle.BaseMsg) {
	info := msg.Data.(hotInfo)
	for _, v := range m.servers {
		if v.stype != ST_LOGIN {
			continue
		}

		pack := network.NewMsgPack(len(info.Lua))
		pack.WriteString([]byte(info.Lua))
		m._net.SendPackMsg(v.connid, handle.IM_LOGIN_AI_UPDATE, pack)
	}
}
