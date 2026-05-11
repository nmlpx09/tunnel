# about

simple udp tunnel without sessions and handshakes

# requirement

1. g++ (version 10.2.1 or newest)
2. make

# confugure env

TUN_DEVICE - tun device name  
REMOTE_IP - server ip  
REMOTE_PORT - server port  
LOCAL_PORT- client port  
TUN_IP - local tun ip  
MTU - mtu (1480 max)  

# command

## build client or server

make

## install client or server

make install

## uninstall client  or server

make uninstall

## start client

tclient.sh start

## stop client

tclient.sh stop

## start server

tserver.sh start

## stop server

tserver.sh stop
