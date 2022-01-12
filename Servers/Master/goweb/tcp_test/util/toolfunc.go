package util

import "time"

func GetSecond() int64 {
	return time.Now().Unix()
}

func GetMillisecond() int64 {
	return time.Now().UnixNano() / 1e6
}

var confdata = make(map[string]string)

func SetConfValue(f string, v string) {
	confdata[f] = v
}

func GetConfValue(f string) string {
	return confdata[f]
}
