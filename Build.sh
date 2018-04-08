#!/bin/bash
if [[ ! -d "./Build" ]] ;then
    mkdir ./Build
fi
cd ./Build

function ReCmake()
{
    rm -rf ./*
    cmake -DCMAKE_BUILD_TYPE=${1} ..
    echo "press any key to make"
    read -n 1
    make
    echo "finish"
}

while getopts "DRBC" VAR
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