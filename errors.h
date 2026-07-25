#pragma once

#include <system_error>

enum EErrorCode {
    Ok,
    KeySize,
    InitKey,
    InitEncrypt,
    InitDecrypt,
    EpollAdd,
    EpollInit,
    EpollZeroInit,
    EpollCreate,
    EpollExit,
    EpollWaitTime,
    SocketOpen,
    SocketBind,
    SocketConfig,
    TunOpen,
    TunBind,
    TunConfig,
    TunDevice,
    TunMtu,
    TunMtuConvert,
    TunMtuMaxSize,
    KeysFile,
    RemoteIp,
    RemotePort,
    RemotePortConvert,
    LocalPort,
    LocalPortConvert,
};

namespace std {

template<> struct is_error_code_enum<EErrorCode> : std::true_type{};

}

class TErrorCategory final : public std::error_category {
public:
    const char* name() const noexcept override {
        return "tunnel error";
    }

    std::string message(int value) const override {
        switch (value) {
            case Ok:
                return "ok";
            case KeySize:
                return "key size";
            case InitKey:
                return "init key";
            case InitEncrypt:
                return "init encrypt";
            case InitDecrypt:
                return "init decrypt";
            case EpollAdd:
                return "epoll add";
            case EpollInit:
                return "epoll init";
            case EpollZeroInit:
                return "epoll events zero init";
            case EpollCreate:
                return "epoll create";
            case EpollExit:
                return "epoll exit";
            case EpollWaitTime:
                return "epoll wait time must be less than 10s";
            case SocketOpen:
                return "socket open";
            case SocketBind:
                return "socket bind";
            case SocketConfig:
                return "socket config";
            case TunOpen:
                return "tun open";
            case TunBind:
                return "tun bind";
            case TunConfig:
                return "tun config";
            case TunDevice:
                return "export TUN_DEVICE env";
            case TunMtu:
                return "export TUN_MTU env";
            case TunMtuConvert:
                return "convert TUN_MTU from string";
            case TunMtuMaxSize:
                return "tun mtu must be less than MAX_TUN_MTU_SIZE";
            case KeysFile:
                return "export KEYS_FILE env";
            case RemoteIp:
                return "export REMOTE_IP env";
            case RemotePort:
                return "export REMOTE_PORT env";
            case RemotePortConvert:
                return "convert REMOTE_PORT from string";
            case LocalPort:
                return "export LOCAL_PORT env";
            case LocalPortConvert:
                return "convert LOCAL_PORT from string";
        }

        return "unknown error code: " + std::to_string(value);
    }

    static const TErrorCategory& instance() {
        static const TErrorCategory errorCategory;
        return errorCategory;
    }
};

inline std::error_code make_error_code(EErrorCode errorCode) {
    return {errorCode, TErrorCategory::instance()};
}
