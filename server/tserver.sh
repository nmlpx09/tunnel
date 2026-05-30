#!/bin/bash

set -ex

TUN_DEVICE=tun0
TUN_IP=10.0.3.1
TUN_MTU=1460

LOCAL_DEVICE=`ip route get 1.1.1.1 | head -1 | cut -d ' ' -f 5`
LOCAL_PORT=69

BIN_NAME=tserver-bin

KEYS_FILE=/etc/tunnel/keys

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

        ip tuntap add mode tun $TUN_DEVICE
        ip address add $TUN_IP/24 dev $TUN_DEVICE
        ip link set dev $TUN_DEVICE mtu $TUN_MTU
        ip link set dev $TUN_DEVICE up

        sysctl net.ipv4.ip_forward=1

        iptables -t nat -A POSTROUTING -s $TUN_IP/24 -o $LOCAL_DEVICE -j MASQUERADE

        export TUN_DEVICE=$TUN_DEVICE
        export TUN_MTU=$TUN_MTU
        export LOCAL_PORT=$LOCAL_PORT
        export KEYS_FILE=$KEYS_FILE

        nice --15 $BIN_NAME |& logger -t $BIN_NAME &
        ;;

    "d")
        test_sudo
        ! test_interface $TUN_DEVICE && echo "interface $TUN_DEVICE not exits" && exit 1

        ip link delete $TUN_DEVICE

        sysctl net.ipv4.ip_forward=0

        iptables -t nat -D POSTROUTING -s $TUN_IP/24 -o $LOCAL_DEVICE -j MASQUERADE

        pkill -9 $BIN_NAME
        ;;
    *)
esac
