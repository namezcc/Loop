#!/bin/bash

work_path=$(pwd)
echo "$work_path" >&2

startServer(){
	nohup ./Server -t $2 -n $3 > log/$1-$3.log 2>&1 &
}

startServer $3 $1 $2