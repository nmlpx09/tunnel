#!/bin/bash

set -ex

BIN_NAME=tserver
TUN_DEVICE=tun0
NET_DEVICE=`ip route | grep '^default' | cut -d ' ' -f 5-`
LOCAL_PORT=1234
TUN_IP=10.0.3.1
MTU=1460
KEYS_FILE=/etc/tunnel/keys

function test_sudo {
    if [ `whoami` != root  ]; then
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

        ip tuntap add mode tun $TUN_DEVICE
        ip address add $TUN_IP/24 dev $TUN_DEVICE
        ip link set dev $TUN_DEVICE mtu $MTU
        ip link set dev $TUN_DEVICE up

        sysctl net.ipv4.ip_forward=1

        iptables -t nat -A POSTROUTING -s $TUN_IP/24 -o $NET_DEVICE -j MASQUERADE

        export TUN_DEVICE=$TUN_DEVICE
        export LOCAL_PORT=$LOCAL_PORT
        export MTU=$MTU
        export KEYS_FILE=$KEYS_FILE

        nice --15 $BIN_NAME &
        ;;

    "stop")
        test_sudo
        ! test_interface $TUN_DEVICE && echo "interface $TUN_DEVICE not exits" && exit 1

        ip link delete $TUN_DEVICE

        iptables -t nat -D POSTROUTING -s $TUN_IP/24 -o $NET_DEVICE -j MASQUERADE

        pkill -15 $BIN_NAME
        ;;
    *)
esac
