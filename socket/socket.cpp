#include "socket.h"

#include <errors.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <utility>

namespace NSocket {

TSocket::~TSocket() {
    if (Fd >= 0) {
        close(Fd);
        Fd = -1;
    }
}

TSocket::TSocket(std::size_t maxBufferSize)
: MaxBufferSize(maxBufferSize)
, Buffer(MaxBufferSize, 0) { }

std::error_code TSocket::Init(
    std::uint32_t localIp,
    std::uint16_t localPort
) noexcept {
    if (Fd >= 0) {
        return {};
    }

    if (Fd = socket(AF_INET, SOCK_DGRAM, 0); Fd < 0) {
        return EErrorCode::SocketOpen;
    }

    const sockaddr_in sockaddrClient = sockaddr_in {
        .sin_family = AF_INET,
        .sin_port = htons(localPort),
        .sin_addr = {
            .s_addr = localIp
        },
        .sin_zero = {0}
    };

    if (auto ret = bind(Fd,
        reinterpret_cast<const sockaddr*>(&sockaddrClient),
        sizeof(sockaddrClient)); ret < 0
    ) {
        return EErrorCode::SocketBind;
    }

    if (auto flags = fcntl(Fd, F_GETFL, 0); flags < 0) {
        return EErrorCode::SocketConfig;
    } else {
        if (auto ret = fcntl(Fd, F_SETFL, flags | O_NONBLOCK); ret < 0) {
            return EErrorCode::SocketConfig;
        }
    }

    return {};
}

std::int32_t TSocket::GetFd() const noexcept {
    return Fd;
}

void TSocket::Write(
    TBufferView buffer,
    std::uint32_t remoteIp,
    std::uint16_t remotePort
) const noexcept {
    if (Fd < 0 || buffer.size() == 0) {
        return;
    }

    auto sockaddrRemote = sockaddr_in {
        .sin_family = AF_INET,
        .sin_port = htons(remotePort),
        .sin_addr = {
            .s_addr = remoteIp
        },
        .sin_zero = {0}
    };

    iovec iov[] = {
        { .iov_base = const_cast<std::uint8_t*>(buffer.data()), .iov_len = buffer.size() }
    };

    const msghdr msg = {
        .msg_name = &sockaddrRemote,
        .msg_namelen = sizeof(sockaddrRemote),
        .msg_iov = iov,
        .msg_iovlen = 1,
        .msg_control = nullptr,
        .msg_controllen = 0,
        .msg_flags = 0
    };

    const auto writeSize = sendmsg(Fd, &msg, 0);

    if (writeSize < 0) {
        return;
    }
}

std::tuple<
    TBufferView,
    std::uint32_t,
    std::uint16_t
> TSocket::Read() noexcept {
    if (Fd < 0) {
        return {{}, 0, 0};
    }

    sockaddr_in sockaddrRemote;

    iovec iov[] = {
        { .iov_base = Buffer.data(), .iov_len = MaxBufferSize }
    };

    msghdr msg = {
        .msg_name = &sockaddrRemote,
        .msg_namelen = sizeof(sockaddrRemote),
        .msg_iov = iov,
        .msg_iovlen = 1,
        .msg_control = nullptr,
        .msg_controllen = 0,
        .msg_flags = 0
    };

    const auto readSize = recvmsg(Fd, &msg, 0);

    if (!std::in_range<std::size_t>(readSize)) {
        return {{}, 0, 0};
    }

    return {{Buffer.begin(), static_cast<std::size_t>(readSize)}, sockaddrRemote.sin_addr.s_addr, ntohs(sockaddrRemote.sin_port)};
}

}
