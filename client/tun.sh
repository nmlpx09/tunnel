#!/bin/bash

set -ex

REMOTE_IP=
REMOTE_PORT=69

TUN_DEVICE=tun0
TUN_IP=10.0.3.2
TUN_MTU=1460

BIN_NAME=tun0

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

        ip route add $REMOTE_IP `ip route | grep '^default' | cut -d ' ' -f 2-`
        ip route add 128.0.0.0/1 dev $TUN_DEVICE
        ip route add 0.0.0.0/1 dev $TUN_DEVICE

        export REMOTE_IP=$REMOTE_IP
        export REMOTE_PORT=$REMOTE_PORT
        export TUN_DEVICE=$TUN_DEVICE
        export TUN_MTU=$TUN_MTU
        export KEYS_FILE=$KEYS_FILE

        ./tun
        ;;

    "d")
        test_sudo

        pkill -9 $BIN_NAME || :

        ip link delete $TUN_DEVICE || :

        ip route del $REMOTE_IP  || :
        ;;
    *)
esac
