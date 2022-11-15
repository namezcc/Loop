package util

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"regexp"
	"strconv"
	"time"
)

func GetSecond() int64 {
	return time.Now().Unix()
}

func GetMillisecond() int64 {
	return time.Now().UnixNano() / 1e6
}

func NowTime() string {
	return time.Now().Format("2006-01-02 15:04:05")
}

var confdata = make(map[string]string)

func SetConfValue(f string, v string) {
	confdata[f] = v
}

func GetConfValue(f string) string {
	return confdata[f]
}

func GetConfInt(f string) int {
	res, _ := strconv.Atoi(confdata[f])
	return res
}

func parseLine(str string) {
	reg := regexp.MustCompile("([\\w-]*)=(.*)")
	sarr := reg.FindStringSubmatch(str)
	if len(sarr) == 3 {
		SetConfValue(sarr[1], sarr[2])
	}
}

func Readconf(fname string) {
	f, err := os.Open(fname)
	if err != nil {
		fmt.Println("err=", err)
		return
	}

	defer f.Close()

	reader := bufio.NewReader(f)

	for {
		str, _, err := reader.ReadLine()

		if len(str) > 0 && str[0] != '#' {
			parseLine(string(str))
		}

		if err == io.EOF {
			break
		}
	}

}
