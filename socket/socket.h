#pragma once

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
    TSocket(std::size_t maxBufferSize);
    TSocket(const TSocket&) = delete;
    TSocket(TSocket&&) = delete;
    TSocket& operator=(const TSocket&) = delete;
    TSocket& operator=(TSocket&&) = delete;
    ~TSocket();

    std::error_code Init(std::uint32_t localIp, std::uint16_t localPort);
    std::int32_t GetFd() const;

    void Write(
        TBufferView,
        std::uint32_t remoteIp,
        std::uint16_t remotePort
    ) const noexcept;

    std::tuple<
        TBufferView,
        std::uint32_t,
        std::uint16_t
    > Read() noexcept;

private:
    std::int32_t Fd = -1;
    std::size_t MaxBufferSize = 0;
    TBuffer Buffer;
};

using TSocketPtr = std::shared_ptr<TSocket>;

}
