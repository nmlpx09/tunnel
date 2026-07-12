#include "socket.h"

#include <errors.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

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

    if (auto ret = fcntl(Fd, F_SETFL, O_NONBLOCK); ret < 0) {
        return EErrorCode::SocketConfig;
    }

    return {};
}

std::int32_t TSocket::GetFd() const noexcept {
    return Fd;
}

void TSocket::Write(
    const TBuffer& buffer,
    std::size_t size,
    std::uint32_t remoteIp,
    std::uint16_t remotePort
) const noexcept {
    if (Fd < 0 || size == 0) {
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
        { .iov_base = const_cast<std::uint8_t*>(buffer.data()), .iov_len = size }
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
    std::reference_wrapper<const TBuffer>,
    std::size_t,
    std::uint32_t,
    std::uint16_t
> TSocket::Read() noexcept {
    if (Fd < 0) {
        return {cref(Buffer), 0, {}, 0};
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

    if (readSize <= 0) {
        return {cref(Buffer), 0, {}, 0};
    }

    return {cref(Buffer), readSize, sockaddrRemote.sin_addr.s_addr, ntohs(sockaddrRemote.sin_port)};
}

}
