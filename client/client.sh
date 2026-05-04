#!/bin/bash

set -ex

TUN_DEVICE=tun0
REMOTE_IP=77.91.92.110
TUN_IP=10.0.3.2
MTU=1400

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
    "c")
        test_sudo
        test_interface $TUN_DEVICE && echo "interface $TUN_DEVICE exits" && exit 1

        ip tuntap add mode tun $TUN_DEVICE
        ip address add $TUN_IP/24 dev $TUN_DEVICE
        ip link set dev $TUN_DEVICE mtu 1400
        ip link set dev $TUN_DEVICE up

        ip route add $REMOTE_IP $(ip route | grep '^default' | cut -d ' ' -f 2-)
        ip route add 128.0.0.0/1 dev $TUN_DEVICE
        ip route add 0.0.0.0/1 dev $TUN_DEVICE

        ;;

    "d")
        test_sudo
        ! test_interface $TUN_DEVICE && echo "interface $TUN_DEVICE not exits" && exit 1

        ip link delete $TUN_DEVICE

        ip route del $REMOTE_IP
        ;;
    *)
esac
