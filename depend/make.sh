#!/bin/bash
if [[ ! -d "./Build" ]]
then
    mkdir ./Build
fi

cd ./Build

rm -rf ./*

mkdir ./Debug
mkdir ./Release

libShared="OFF"
libType="Debug"

while getopts "DRSH" arg
do
    case $arg in
        D) libType="Debug"
        ;;
        R) libType="Release"
        ;;
        S) libShared="ON"
        ;;
        H) echo "-D debug -R Release -S Shared(static default)"
        ;;
    esac
done

cmake3 -DCMAKE_BUILD_TYPE=${libType} -DBUILD_SHARED_LIBS=${libShared} ..
echo "press any key to make"
read -n 1
make
echo "success"