#!/bin/bash

set -ex

REMOTE_IP=77.91.92.110
REMOTE_PORT=69

TUN_DEVICE=tun0
TUN_IP=10.0.3.3
TUN_MTU=1460

LOCAL_DEVICE=`ip route get 1.1.1.1 | head -1 | cut -d ' ' -f 5`

BIN_NAME=tc0

KEYS_FILE=/data/data/com.termux/files/usr/etc/tunnel/keys

function test_sudo {
    if [ `whoami` != root ]; then
        echo "run on sudo"

        exit 1
    fi
}

function test_interface {
    ip link show $1 &> /dev/null || return 1

    return 0
}

case $1 in
    "c")
        test_sudo

        test_interface $TUN_DEVICE && echo "interface $TUN_DEVICE exits" && exit 1
        ! test_interface $LOCAL_DEVICE && echo "interface $LOCAL_DEVICE not exits" && continue

        ip tuntap add mode tun $TUN_DEVICE
        ip address add $TUN_IP/24 dev $TUN_DEVICE
        ip link set dev $TUN_DEVICE mtu $TUN_MTU
        ip link set dev $TUN_DEVICE up

        ip route add table $LOCAL_DEVICE $REMOTE_IP `ip route show table $LOCAL_DEVICE | grep '^default' | cut -d ' ' -f 2-`
        ip route add table $LOCAL_DEVICE 128.0.0.0/1 dev $TUN_DEVICE
        ip route add table $LOCAL_DEVICE 0.0.0.0/1 dev $TUN_DEVICE

        export REMOTE_IP=$REMOTE_IP
        export REMOTE_PORT=$REMOTE_PORT
        export TUN_DEVICE=$TUN_DEVICE
        export TUN_MTU=$TUN_MTU
        export KEYS_FILE=$KEYS_FILE

        setsid nice --15 $BIN_NAME
        ;;

    "d")
        test_sudo
        ! test_interface $TUN_DEVICE && echo "interface $TUN_DEVICE not exits" && exit 1

        pkill -9 $BIN_NAME

        ip link delete $TUN_DEVICE

        ip route del table $LOCAL_DEVICE $REMOTE_IP
        ;;
    *)
esac
