package module

import (
	"fmt"
	"net/http"
	"strconv"
	"sync"
	"tcp_test/handle"
	"tcp_test/network"
	"tcp_test/util"
	"time"

	"github.com/gin-gonic/gin"
	_ "github.com/go-sql-driver/mysql"
	"github.com/jmoiron/sqlx"
)

type responesData struct {
	index int
	data  interface{}
}

type msgResponse struct {
	_c       chan *responesData
	_endtime int64
	_id      int
}

type HttpModule struct {
	modulebase
	_msg_response   map[int]msgResponse
	_rsp_index      int
	_rsp_index_lock sync.Mutex
	_log_mod        *MasterModule
	_net_mod        *netModule
	_sql            *sqlx.DB
}

func (m *HttpModule) Init(mgr *moduleMgr) {
	m._mod_mgr = mgr
	m._msg_response = make(map[int]msgResponse)
	m._rsp_index = 0
	m._net_mod = mgr.GetModule(MOD_NET).(*netModule)
	m._log_mod = mgr.GetModule(MOD_MASTER).(*MasterModule)

	handle.Handlemsg.AddMsgCall(handle.N_WBE_ON_RESPONSE, m.onMsgRespones)

	host := util.GetConfValue("mysql")

	sql, sqlerr := sqlx.Open("mysql", host)
	m._sql = sql

	if sqlerr != nil {
		panic("sql open error:" + sqlerr.Error())
	}

	sql.SetMaxOpenConns(100)
	// 闲置连接数
	sql.SetMaxIdleConns(20)
	// 最大连接周期
	sql.SetConnMaxLifetime(100 * time.Second)
}

func Cors() gin.HandlerFunc {
	return func(c *gin.Context) {
		origin := c.Request.Header.Get("origin") //请求头部
		if len(origin) == 0 {
			origin = c.Request.Header.Get("Origin")
		}
		//接收客户端发送的origin （重要！）
		c.Writer.Header().Set("Access-Control-Allow-Origin", origin)
		//允许客户端传递校验信息比如 cookie (重要)
		c.Writer.Header().Set("Access-Control-Allow-Credentials", "true")
		c.Writer.Header().Set("Access-Control-Allow-Headers", "Content-Type, Content-Length, Accept-Encoding, X-CSRF-Token, Authorization, accept, origin, Cache-Control, X-Requested-With")
		//服务器支持的所有跨域请求的方法
		c.Writer.Header().Set("Access-Control-Allow-Methods", "OPTIONS, GET, POST, PUT, DELETE, UPDATE")
		c.Writer.Header().Set("Content-Type", "application/json; charset=utf-8")
		// 设置预验请求有效期为 86400 秒
		c.Writer.Header().Set("Access-Control-Max-Age", "86400")
		if c.Request.Method == "OPTIONS" {
			c.AbortWithStatus(204)
			return
		}
		c.Next()
	}
}

func (m *HttpModule) AfterInit() {

	router := gin.Default()

	router.Use(Cors())

	router.GET("/", m.apiIndex)
	router.GET("/serverInfo", m.getServerConf)
	router.GET("/machine", m.getMachine)
	router.GET("/viewMachine", m.viewMachine)
	router.GET("/getServer", m.getServerInfo)
	router.GET("/serverOpt", m.serverOpt)

	go func() {
		router.Run(":8999")
		fmt.Println("start http server")
	}()

	m.setTikerFunc(time.Second*5, m.checkRespones)
}

func (m *HttpModule) requestMsg(mod modulebase, mid int, d interface{}) *responesData {
	mrsp := m.getRespones()

	mod.sendMsg(mid, responesData{
		index: mrsp._id,
		data:  d,
	})

	getpack := <-mrsp._c
	return getpack
}

func (m *HttpModule) getRespones() msgResponse {
	m._rsp_index_lock.Lock()
	defer m._rsp_index_lock.Unlock()
	msg := msgResponse{
		_c:       make(chan *responesData),
		_endtime: util.GetMillisecond() + 500000, //5s
		_id:      m._rsp_index,
	}
	m._msg_response[m._rsp_index] = msg
	m._rsp_index++
	return msg
}

func (m *HttpModule) checkRespones(dt int64) {
	m._rsp_index_lock.Lock()
	defer m._rsp_index_lock.Unlock()

	for k, v := range m._msg_response {
		if dt > v._endtime {
			v._c <- nil
			delete(m._msg_response, k)
		}
	}
}

func (m *HttpModule) onMsgRespones(msg handle.BaseMsg) {
	pack := msg.Data.(responesData)
	index := pack.index
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

func (m *HttpModule) apiIndex(c *gin.Context) {
	fmt.Println(c.Request.Method, c.Request.URL)

	pack := m.requestMsg(m._log_mod.modulebase, handle.N_WBE_REQUEST_1, nil)
	if pack == nil {
		c.String(http.StatusOK, "getpack nil")
		return
	}

	c.String(http.StatusOK, string(""))
}

type dbMachine struct {
	Id int    `db:"id" json:"id"`
	Ip string `db:"ip" json:"ip"`
}

type dbServer struct {
	Type      int         `db:"type" json:"type"`
	Id        int         `db:"id" json:"id"`
	Name      string      `db:"name" json:"name"`
	Group     int         `db:"group" json:"group"`
	Port      int         `db:"port" json:"port"`
	Mysql     string      `db:"mysql" json:"mysql"`
	Redis     string      `db:"redis" json:"redis"`
	MysqlHost dbMysqlHost `json:"mysqlhost"`
}

type dbMysqlHost struct {
	Id     int    `db:"id" json:"id"`
	Ip     string `db:"ip" json:"ip"`
	Port   int    `db:"port" json:"port"`
	Dbname string `db:"dbname" json:"dbname"`
	User   string `db:"user" json:"user"`
	Pass   string `db:"pass" json:"pass"`
}

type dbServiceFind struct {
	Id   int    `db:"id" json:"id"`
	Ip   string `db:"ip" json:"ip"`
	Port int    `db:"port" json:"port"`
}

type ServiceInfo struct {
	Server  dbServer        `json:"server"`
	Service []dbServiceFind `json:"service"`
}

func (m *HttpModule) getServerConf(c *gin.Context) {
	var serviceinfo ServiceInfo

	id, _ := strconv.Atoi(c.Query("id"))
	type_, _ := strconv.Atoi(c.Query("type"))

	err := m._sql.Get(&serviceinfo.Server, "SELECT * FROM `server` WHERE type=? AND id=?;", type_, id)
	if err != nil {
		fmt.Println(err)
		c.String(http.StatusOK, err.Error())
		return
	}

	if serviceinfo.Server.Mysql != "" {
		err = m._sql.Get(&serviceinfo.Server.MysqlHost, "SELECT * FROM `mysql_host` WHERE `id`=?;", serviceinfo.Server.Mysql)
		if err != nil {
			fmt.Println(err)
			c.String(http.StatusOK, err.Error())
			return
		}
	}

	err = m._sql.Select(&serviceinfo.Service, "SELECT * FROM `service_find`")
	if err != nil {
		fmt.Println(err)
		c.String(http.StatusOK, err.Error())
		return
	}

	c.JSON(http.StatusOK, serviceinfo)
}

func (m *HttpModule) getMachine(c *gin.Context) {

	var machines []dbMachine
	err := m._sql.Select(&machines, "select * from machine;")
	if err != nil {
		c.String(http.StatusOK, err.Error())
		return
	}

	// js, _ := json.Marshal(machines)
	// c.String(http.StatusOK, "")
	c.JSON(http.StatusOK, machines)

}

type servernode struct {
	Type  int32  `json:"type"`
	Id    int32  `json:"id"`
	Name  string `json:"name"`
	Open  int32  `json:"open"`
	Error int32  `json:"error"`
}

type machineInfo struct {
	Open   int32        `json:"open"`
	Server []servernode `json:"server"`
}

var serverName = map[int32]string{
	1:  "proxy",
	2:  "login",
	3:  "mysqlServer",
	4:  "proxy",
	7:  "dbproxy",
	8:  "room",
	9:  "room mgr",
	10: "loginLock",
	15: "team",
	17: "account_db",
	18: "teamproxy",
}

func (m *HttpModule) viewMachine(c *gin.Context) {

	minfo := machineInfo{
		Open: 0,
	}

	id, _ := strconv.Atoi(c.Query("id"))
	if id == 0 {
		c.JSON(http.StatusOK, minfo)
		return
	}

	spack := network.NewMsgPackDef()
	spack.WriteInt32(id)

	pack := m.requestMsg(m._log_mod.modulebase, handle.N_WEB_VIEW_MACHINE, &spack)
	if pack == nil {
		c.JSON(http.StatusOK, minfo)
		return
	}

	// minfo.Open = pack.ReadInt32()

	// num := pack.ReadInt32()

	// for i := 0; i < int(num); i++ {
	// 	type_ := pack.ReadInt32()
	// 	id_ := pack.ReadInt32()
	// 	open_ := pack.ReadInt32()
	// 	err_ := pack.ReadInt32()

	// 	minfo.Server = append(minfo.Server, servernode{
	// 		Type:  type_,
	// 		Name:  serverName[type_],
	// 		Id:    id_,
	// 		Open:  open_,
	// 		Error: err_,
	// 	})
	// }

	c.JSON(http.StatusOK, minfo)
}

type serLink struct {
	Name string
	Type int32
	Id   int32
	Port int32
	Open int32
}

type serInfo struct {
	Link []serLink
}

func (m *HttpModule) getServerInfo(c *gin.Context) {

	mid, _ := strconv.Atoi(c.Query("mid"))
	sertype, _ := strconv.Atoi(c.Query("type"))
	serid, _ := strconv.Atoi(c.Query("id"))

	spack := network.NewMsgPackDef()
	spack.WriteInt32(mid)
	spack.WriteInt32(sertype)
	spack.WriteInt32(serid)

	var vserInfo serInfo

	pack := m.requestMsg(m._log_mod.modulebase, handle.N_WEB_GET_SERVER_INFO, &spack)
	if pack == nil {
		c.JSON(http.StatusOK, vserInfo)
		return
	}

	// num := pack.ReadInt32()

	// for i := 0; i < int(num); i++ {

	// 	lst := pack.ReadInt32()
	// 	vserInfo.Link = append(vserInfo.Link, serLink{
	// 		Name: serverName[lst],
	// 		Type: lst,
	// 		Id:   pack.ReadInt32(),
	// 		Port: pack.ReadInt32(),
	// 		Open: pack.ReadInt32(),
	// 	})
	// }

	c.JSON(http.StatusOK, vserInfo)
}

type hotInfo struct {
	Lua    string `json:"lua"`
	Server []int  `json:"server"`
	Except []int  `json:"except"`
}

func (m *HttpModule) serverOpt(c *gin.Context) {

	mid, _ := strconv.Atoi(c.Query("mid"))
	sertype, _ := strconv.Atoi(c.Query("type"))
	serid, _ := strconv.Atoi(c.Query("id"))
	opt, _ := strconv.Atoi(c.Query("opt"))

	spack := network.NewMsgPackDef()
	spack.WriteInt32(mid)
	spack.WriteInt32(sertype)
	spack.WriteInt32(serid)
	spack.WriteInt32(opt)

	m.sendMsg(handle.N_WEB_SERVER_OPT, &spack)
}
