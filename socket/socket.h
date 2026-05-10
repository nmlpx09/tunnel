#pragma once

#include <common/types.h>
#include <epoll/epoll.h>

#include <cstdint>
#include <memory>
#include <string>
#include <tuple>

namespace NSocket {

struct TSocket {
public:
    TSocket(std::size_t maxBufferSize) noexcept;
    TSocket(const TSocket&) = delete;
    TSocket(TSocket&&) = delete;
    TSocket& operator=(const TSocket&) = delete;
    TSocket& operator=(TSocket&&) = delete;
    ~TSocket();

    std::int32_t Init(std::string localHost, std::uint16_t localPort) noexcept;

    void Write(
        const TBuffer& buffer,
        std::size_t size,
        const std::string& remoteHost,
        std::uint16_t remotePort
    ) const noexcept;

    std::tuple<std::size_t, std::string, std::uint16_t> Read() noexcept;

    const TBuffer& GetBuffer() const noexcept;

    bool IsFd(std::int32_t fd) const noexcept;

private:
    std::int32_t Fd = -1;
    std::size_t MaxBufferSize = 0;
    TBuffer Buffer;
    friend class NEpoll::TEpoll;
};

using TSocketPtr = std::shared_ptr<TSocket>;

}
