#!/bin/bash

set -ex

TUN_DEVICE=tun0
REMOTE_IP=77.91.92.110

if [ `whoami` != root ]; then
    echo "run on sudo"
    exit 1
fi

ip link delete $TUN_DEVICE

ip route del $REMOTE_IP
