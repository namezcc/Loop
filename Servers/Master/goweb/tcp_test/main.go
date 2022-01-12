package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"regexp"
	"tcp_test/module"
	"tcp_test/util"
	"time"
)

func parseLine(str string) {
	reg := regexp.MustCompile("(\\w*)=(.*)")
	sarr := reg.FindStringSubmatch(str)
	if len(sarr) == 3 {
		util.SetConfValue(sarr[1], sarr[2])
		// fmt.Println("host ", host)
	}
}

func readconf() {
	// fmt.Println("read file start")

	f, err := os.Open("conf.ini")
	if err != nil {
		fmt.Println("err=", err)
		return
	}

	defer f.Close()

	reader := bufio.NewReader(f)

	for {
		str, _, err := reader.ReadLine()

		if len(str) > 0 {
			parseLine(string(str))
		}

		if err == io.EOF {
			break
		}
	}

}

func main() {
	readconf()

	module.ServerAddr = util.GetConfValue("host")
	module.ModuleMgr.Init()
	fmt.Println("start main")
	module.ModuleMgr.Run()
	for range time.Tick(time.Second * 60) {

	}
}
