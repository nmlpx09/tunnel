#!/bin/bash

set -ex

BIN_NAME=tclient
TUN_DEVICE=tun0
REMOTE_IP=77.91.92.110
DEVICE=`ip route get $REMOTE_IP | head -1 | awk '{print $5}'`
REMOTE_PORT=69
LOCAL_PORT=1234
TUN_IP=10.0.3.3
MTU=1460
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
    "start")
        test_sudo

        test_interface $TUN_DEVICE && echo "interface $TUN_DEVICE exits" && exit 1
        ! test_interface $DEVICE && echo "interface $DEVICE not exits" && continue

        ip tuntap add mode tun $TUN_DEVICE
        ip address add $TUN_IP/24 dev $TUN_DEVICE
        ip link set dev $TUN_DEVICE mtu $MTU
        ip link set dev $TUN_DEVICE up

        ip route add table $DEVICE $REMOTE_IP `ip route show table $DEVICE | grep '^default' | cut -d ' ' -f 2-`
        ip route add table $DEVICE 128.0.0.0/1 dev $TUN_DEVICE
        ip route add table $DEVICE 0.0.0.0/1 dev $TUN_DEVICE

        export TUN_DEVICE=$TUN_DEVICE
        export REMOTE_IP=$REMOTE_IP
        export REMOTE_PORT=$REMOTE_PORT
        export LOCAL_PORT=$LOCAL_PORT
        export MTU=$MTU
        export KEYS_FILE=$KEYS_FILE

        $BIN_NAME
        ;;

    "stop")
        test_sudo
        ! test_interface $TUN_DEVICE && echo "interface $TUN_DEVICE not exits" && exit 1

        ip link delete $TUN_DEVICE

        ip route del table $DEVICE $REMOTE_IP
        ;;
    *)
esac
