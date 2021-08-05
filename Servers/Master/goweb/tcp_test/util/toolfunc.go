package util

import "time"

func GetSecond() int64 {
	return time.Now().Unix()
}

func GetMillisecond() int64 {
	return time.Now().UnixNano() / 1e6
}
