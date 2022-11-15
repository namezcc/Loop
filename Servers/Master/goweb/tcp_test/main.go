package main

import (
	"fmt"
	"tcp_test/handle"
	"tcp_test/module"
	"tcp_test/util"
	"time"
)

func main() {
	util.Readconf("conf.ini")

	module.ServerAddr = util.GetConfValue("host")
	module.ModuleMgr.Init()
	fmt.Println("start main")

	module.ModuleMgr.InitMsg(handle.MS_BEGIN, handle.MS_END)

	module.ModuleMgr.AddModule(module.MOD_MASTER, &module.MasterModule{})
	module.ModuleMgr.AddModule(module.MOD_HTTP, &module.HttpModule{})

	module.ModuleMgr.StartRun()
	for range time.Tick(time.Second * 60) {

	}
}
