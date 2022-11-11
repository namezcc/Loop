FROM centos:7.9.2009
COPY user.conf /etc/ld.so.conf.d/
COPY Debug /root/server/
COPY syslib /root/server/
RUN ldconfig &&\
    /bin/cp /usr/share/zoneinfo/Asia/Shanghai /etc/localtime &&\
    echo 'Asia/Shanghai' >/etc/timezone
WORKDIR /root/server
CMD ["./Server","-t","2","-n","1"]
