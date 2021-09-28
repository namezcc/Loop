#!/bin/bash

ps -ef|grep "Server -t $1 -n $2"|grep -v grep|awk '{print $2}'