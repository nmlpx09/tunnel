#pragma once

#include <system_error>

enum EErrorCode {
    Ok,
    InitEncrypt,
    InitDecrypt,
    EpollAdd,
    EpollZeroInit,
    EpollCreate,
    SocketOpen,
    SocketBind,
    SocketConfig,
    TunOpen,
    TunBind,
    TunConfig,
};

namespace std {

template<> struct is_error_code_enum<EErrorCode> : std::true_type{};

}

class TErrorCategory final : public std::error_category {
public:
    const char* name() const noexcept override {
        return "tnnel error";
    }

    std::string message(int value) const override {
        switch (value) {
            case Ok:
                return "ok";
            case InitEncrypt:
                return "init encrypt error";
            case InitDecrypt:
                return "init decrypt error";
            case EpollAdd:
                return "epoll add error";
            case EpollZeroInit:
                return "epoll zero init error";
            case EpollCreate:
                return "epoll create error";
            case SocketOpen:
                return "socket open error";
            case SocketBind:
                return "socket bind error";
            case SocketConfig:
                return "socket config error";
            case TunOpen:
                return "tun open error";
            case TunBind:
                return "tun bind error";
            case TunConfig:
                return "tun config error";
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
