package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"regexp"
	"tcp_test/module"
	"time"
)

var host = ""

func parseLine(str string) {
	reg := regexp.MustCompile("host=(.*)")
	sarr := reg.FindStringSubmatch(str)
	if len(sarr) == 2 {
		host = sarr[1]
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
		str, err := reader.ReadString('\n')

		if str != "" {
			parseLine(str)
		}

		if err == io.EOF {
			break
		}
	}

}

func main() {
	readconf()

	module.ServerAddr = host
	module.ModuleMgr.Init()
	fmt.Println("start main")
	for {
		tm := time.Now().UnixNano() / 1e6
		module.ModuleMgr.Update(tm)
		time.Sleep(time.Millisecond * 1)
	}
}
