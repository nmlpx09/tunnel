#pragma once

#include <poll/poll.h>
#include <types.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <system_error>
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

    std::error_code Init(const std::string& localIp, std::uint16_t localPort) noexcept;

    void Write(
        const TBuffer& buffer,
        std::size_t size,
        const std::string& remoteIp,
        std::uint16_t remotePort
    ) const noexcept;

    std::tuple<
        std::reference_wrapper<const TBuffer>,
        std::size_t,
        std::string,
        std::uint16_t
    > Read() noexcept;

private:
    std::int32_t Fd = -1;
    std::size_t MaxBufferSize = 0;
    TBuffer Buffer;
    friend struct NPoll::TPoll;
};

using TSocketPtr = std::shared_ptr<TSocket>;

}
