#!/bin/bash
if [[ ! -d "./Build" ]] ;then
    mkdir ./Build
fi
cd ./Build

function ReCmake()
{
    rm -rf ./*
    cmake -DCMAKE_BUILD_TYPE=${1} ..
    echo -n "make[y/n]?"
    while read ans
    do
			case $ans in
				Y|y) make
				break
				;;
				N|n) break;;
				*) echo -n "make[y/n]?";;
			esac
    done
    echo "finish"
}

while getopts "DRB" VAR
do
    case $VAR in
        D) ReCmake "Debug"
        ;;
        R) ReCmake "Release"
        ;;
        B) make
        ;;
        *) echo "-D cmake Debug -R cmake Release -B make "
        ;;
    esac
done