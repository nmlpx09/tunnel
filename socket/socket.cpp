#include "socket.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

namespace NSocket {

TSocket::~TSocket() {
    close(Fd);
    Fd = -1;
}

TSocket::TSocket(std::size_t maxBufferSize) noexcept
    : MaxBufferSize(maxBufferSize)
    , Buffer(MaxBufferSize, 0) { }

std::int32_t TSocket::Init(
    std::string localHost,
    std::uint16_t localPort
) noexcept {
    if (Fd = socket(AF_INET, SOCK_DGRAM, 0); Fd < 0) {
        return -1;
    }

    sockaddr_in sockaddrClient = sockaddr_in {
        .sin_family = AF_INET,
        .sin_port = htons(localPort),
        .sin_addr = {
            .s_addr = inet_addr(localHost.c_str())
        },
        .sin_zero = {0}
    };

    if (auto ret = bind(Fd,
        reinterpret_cast<const sockaddr*>(&sockaddrClient),
        sizeof(sockaddrClient)); ret < 0
    ) {
        return -1;
    }

    if (auto ret = fcntl(Fd, F_SETFL, O_NONBLOCK); ret < 0){
        return -1;
    }

    return 0;
}

void TSocket::Write(
    const TBuffer& buffer,
    std::size_t size,
    const std::string& remoteHost,
    std::uint16_t remotePort
) const noexcept {
    if (Fd < 0) {
        return;
    }

    if (size == 0) {
        return;
    }

    auto sockaddrRemote = sockaddr_in {
        .sin_family = AF_INET,
        .sin_port = htons(remotePort),
        .sin_addr = {
            .s_addr = inet_addr(remoteHost.c_str())
        },
        .sin_zero = {0}
    };

    sendto(Fd, buffer.data(), size, 0,
        reinterpret_cast<const sockaddr*>(&sockaddrRemote), sizeof(sockaddrRemote));
}

std::tuple<std::size_t, std::string, std::uint16_t> TSocket::Read() noexcept {
    if (Fd < 0) {
        return {0, {}, 0};
    }

    sockaddr_in sockaddrRemote;
    std::uint32_t sockaddrSize;

    const auto readSize = recvfrom(Fd, Buffer.data(), MaxBufferSize, 0, reinterpret_cast<sockaddr*>(&sockaddrRemote), &sockaddrSize);

    if (readSize <= 0 || sockaddrSize <= 0) {
        return {0, {}, 0};
    }

    return {readSize, inet_ntoa(sockaddrRemote.sin_addr), htons(sockaddrRemote.sin_port)};
}

const TBuffer& TSocket::GetBuffer() const noexcept {
    return Buffer;
}

bool TSocket::IsFd(std::int32_t fd) const noexcept {
    return Fd == fd;
}

}
