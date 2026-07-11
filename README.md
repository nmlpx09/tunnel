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
TUN_MTU - mtu (1460 max)  
REMOTE_IP - server ip  
REMOTE_PORT - remote server port  
LOCAL_PORT- local server port  
KEYS_FILE - file with key and iv in base64  

# command

## build client or server

make

## install client or server

make install

## uninstall client  or server

make uninstall

## install termux

make install_termux

## uninstall termux

make uninstall_termux

## start

tun c

## stop

tun d
