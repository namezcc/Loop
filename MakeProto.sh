for pro in `ls ./proto/client`
do
    protoc -I=./proto/client --cpp_out=dllexport_decl=LIBPROTOC_EXPORT:./protoPB/client/ ${pro}
done

for pro in `ls ./proto/server`
do
    protoc -I=./proto/server --cpp_out=dllexport_decl=LIBPROTOC_EXPORT:./protoPB/server/ ${pro}
done

echo "success"