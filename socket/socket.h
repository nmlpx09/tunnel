#pragma once

#include <types.h>

#include <cstdint>
#include <expected>
#include <functional>
#include <memory>
#include <string>
#include <system_error>
#include <tuple>

namespace NSocket {

struct TReadResult {
    TBufferView buffer;
    std::uint32_t Ip = 0;
    std::uint16_t Port = 0;
};

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

    std::error_code Write(
        TBufferView,
        std::uint32_t remoteIp,
        std::uint16_t remotePort
    ) const noexcept;

    std::expected<TReadResult, std::error_code> Read() noexcept;

private:
    std::int32_t Fd = -1;
    std::size_t MaxBufferSize = 0;
    TBuffer Buffer;
};

using TSocketPtr = std::shared_ptr<TSocket>;

}
