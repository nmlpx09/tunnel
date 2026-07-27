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
) {
    if (Fd >= 0) {
        return {};
    }

    if (Fd = socket(AF_INET, SOCK_DGRAM, 0); Fd < 0) {
        return EErrorCode::SocketOpen;
    }

    const sockaddr_in sockaddrClient = sockaddr_in {
        .sin_family = AF_INET,
        .sin_port = localPort,
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

    const auto socketBufferSize = 2 * 1024 * 1024;
    socklen_t optlen = sizeof(socketBufferSize);

    if (auto ret = setsockopt(Fd, SOL_SOCKET, SO_RCVBUF, &socketBufferSize, optlen); ret < 0) {
        return EErrorCode::SocketConfig;
    }

    if (auto ret = setsockopt(Fd, SOL_SOCKET, SO_SNDBUF, &socketBufferSize, optlen); ret < 0) {
        return EErrorCode::SocketConfig;
    }

    return {};
}

std::int32_t TSocket::GetFd() const {
    return Fd;
}

std::error_code TSocket::Write(
    TBufferView buffer,
    std::uint32_t remoteIp,
    std::uint16_t remotePort
) const noexcept {
    if (Fd < 0) {
        return EErrorCode::SocketOpen;
    }

    if (buffer.empty()) {
        return {};
    }

    auto sockaddrRemote = sockaddr_in {
        .sin_family = AF_INET,
        .sin_port = remotePort,
        .sin_addr = {
            .s_addr = remoteIp
        },
        .sin_zero = {0}
    };

    iovec iov[] = {
        { .iov_base = buffer.data(), .iov_len = buffer.size() }
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

    if (writeSize < 0 || static_cast<std::size_t>(writeSize) != buffer.size()) {
        return EErrorCode::SocketWrite;
    }

    return {};
}

std::expected<TReadResult, std::error_code> TSocket::Read() noexcept {
    if (Fd < 0) {
        return std::unexpected(EErrorCode::SocketOpen);
    }

    auto sockaddrRemote = sockaddr_in {
        .sin_family = AF_INET,
        .sin_port = 0,
        .sin_addr = {
            .s_addr = 0
        },
        .sin_zero = {0}
    };

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

    if (readSize < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return TReadResult{{}, 0, 0};
        }
        return std::unexpected(EErrorCode::SocketRead);
    }

    if (readSize == 0) {
        return TReadResult{{}, 0, 0};
    }

    if (msg.msg_namelen < sizeof(sockaddr_in)) {
        return std::unexpected(EErrorCode::SocketRead);
    }

    return TReadResult{
        {Buffer.begin(), static_cast<std::size_t>(readSize)},
        sockaddrRemote.sin_addr.s_addr,
        sockaddrRemote.sin_port
    };
}

}
