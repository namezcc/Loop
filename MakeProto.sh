#!/bin/bash
makecli(){
	for pro in `ls ./proto/client`
	do
			protoc -I=./proto/client --cpp_out=dllexport_decl=LIBPROTOC_EXPORT:./protoPB/client/ ${pro}
	done
}

makeser(){
	for pro in `ls ./proto/server`
	do
			protoc -I=./proto/server --cpp_out=dllexport_decl=LIBPROTOC_EXPORT:./protoPB/server/ ${pro}
	done
}

makebase(){
	for pro in `ls ./proto/base`
	do
			protoc -I=./proto/base --cpp_out=dllexport_decl=LIBPROTOC_EXPORT:./protoPB/base/ ${pro}
	done
}

if [ ! -n "$1" ]
then
	makecli
	makeser
else
	while getopts bsc opt; do
		case $opt in
			b) makebase;;
			s) makeser;;
			c) makecli;;
			\?) echo -e " -b make base\n -s make server\n -c make client\n";;
		esac
	done
fi

echo "success"