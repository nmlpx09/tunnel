# about

simple udp tunnel with aes128 crypt and without sessions or handshakes

# requirement

1. g++ (version 10.2.1 or newest)
2. make
3. libssl-dev

# allows sytems

1. linux
2. rooted android with termux

# confugure env

TUN_DEVICE - tun device name  
REMOTE_IP - server ip  
REMOTE_PORT - server port  
LOCAL_PORT- client port  
TUN_IP - local tun ip  
MTU - mtu (1460 max)  
KEYS_FILE - file with key and iv  

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

## install termux

make install_termux

## uninstall termux

make uninstall_termux
