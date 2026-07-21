#!/bin/bash

set -exu

REMOTE_IP=158.255.0.70
REMOTE_PORT=69

TUN_DEVICE=tun0
TUN_IP=10.0.3.2
TUN_MTU=1440

BIN_NAME=tun0
PID_FILE=/run/$BIN_NAME.pid

KEYS_FILE=/etc/tunnel/keys

function check_sudo {
    if [ $EUID -ne 0 ]; then
        echo "run on sudo"
        exit 1
    fi
}

function check_interface {
    ip link show $TUN_DEVICE &> /dev/null || return 1
    return 0
}

function add_rules {
    ip tuntap add mode tun $TUN_DEVICE
    ip address add $TUN_IP/24 dev $TUN_DEVICE
    ip link set dev $TUN_DEVICE mtu $TUN_MTU
    ip link set dev $TUN_DEVICE up

    ip route add $REMOTE_IP `ip route | grep '^default' | cut -d ' ' -f 2-`
    ip route add 128.0.0.0/1 dev $TUN_DEVICE
    ip route add 0.0.0.0/1 dev $TUN_DEVICE
}

function remove_rules {
    ip route del $REMOTE_IP
    ip link delete $TUN_DEVICE
}

function check_vars {
    local empty_vars=()

    [[ -z $REMOTE_IP ]]    && empty_vars+=(REMOTE_IP)
    [[ -z $REMOTE_PORT ]]  && empty_vars+=(REMOTE_PORT)
    [[ -z $TUN_DEVICE ]]   && empty_vars+=(TUN_DEVICE)
    [[ -z $TUN_IP ]]       && empty_vars+=(TUN_IP)
    [[ -z $TUN_MTU ]]      && empty_vars+=(TUN_MTU)
    [[ -z $KEYS_FILE ]]    && empty_vars+=(KEYS_FILE)

    if [[ ${#empty_vars[@]} -gt 0 ]]; then
        echo "empty vars: ${empty_vars[*]}"
        exit 1
    fi
}

check_sudo
check_vars

case $1 in
    "c")

        check_interface && echo "interface $TUN_DEVICE exists" && exit 1

        add_rules

        export REMOTE_IP=$REMOTE_IP
        export REMOTE_PORT=$REMOTE_PORT
        export TUN_DEVICE=$TUN_DEVICE
        export TUN_MTU=$TUN_MTU
        export KEYS_FILE=$KEYS_FILE

        start-stop-daemon --start --background \
            --make-pidfile --pidfile $PID_FILE \
            --nicelevel -15 \
            --exec /usr/bin/$BIN_NAME

        sleep 1

        if [ ! -f $PID_FILE ] || ! ps -p `cat $PID_FILE` > /dev/null ; then
            echo "tun not start"
            remove_rules
            exit 1
        fi
        ;;

    "d")
        ! check_interface && echo "interface $TUN_DEVICE not exists" && exit 1

        start-stop-daemon --stop --signal 2 \
            --pidfile $PID_FILE --remove-pidfile || :

        remove_rules || :
        ;;
    *)
        echo "Usage: $0 {c|d}"
        ;;
esac
