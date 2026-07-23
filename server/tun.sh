#!/bin/bash

set -exu

TUN_DEVICE=tun0
TUN_IP=10.0.3.1
TUN_MTU=1444

LOCAL_DEVICE=`ip route get 1.1.1.1 | head -1 | cut -d ' ' -f 5`
LOCAL_PORT=69

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

    sysctl net.ipv4.ip_forward=1

    iptables -t nat -A POSTROUTING -s $TUN_IP/24 -o $LOCAL_DEVICE -j MASQUERADE
}

function remove_rules {
    iptables -t nat -D POSTROUTING -s $TUN_IP/24 -o $LOCAL_DEVICE -j MASQUERADE
    sysctl net.ipv4.ip_forward=0
    ip link delete $TUN_DEVICE
}

function check_vars {
    local empty_vars=()

    [[ -z $TUN_DEVICE ]]    && empty_vars+=(TUN_DEVICE)
    [[ -z $TUN_IP ]]        && empty_vars+=(TUN_IP)
    [[ -z $TUN_MTU ]]       && empty_vars+=(TUN_MTU)
    [[ -z $LOCAL_DEVICE ]]  && empty_vars+=(LOCAL_DEVICE)
    [[ -z $LOCAL_PORT ]]    && empty_vars+=(LOCAL_PORT)
    [[ -z $KEYS_FILE ]]     && empty_vars+=(KEYS_FILE)

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

        export TUN_DEVICE=$TUN_DEVICE
        export TUN_MTU=$TUN_MTU
        export LOCAL_PORT=$LOCAL_PORT
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

    "r")
        ! check_interface && echo "interface $TUN_DEVICE not exists" && exit 1

        start-stop-daemon --stop --signal 2 \
            --pidfile $PID_FILE --remove-pidfile

        sleep 3

        export TUN_DEVICE=$TUN_DEVICE
        export TUN_MTU=$TUN_MTU
        export LOCAL_PORT=$LOCAL_PORT
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
    *)
        echo "Usage: $0 {c|d|r}"
        ;;
esac
