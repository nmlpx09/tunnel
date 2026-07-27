#pragma once

#include <types.h>

#include <cstdint>
#include <expected>
#include <functional>
#include <memory>
#include <string>
#include <system_error>

namespace NTun {

struct TTun {
public:
    TTun(std::size_t maxBufferSize);
    TTun(const TTun&) = delete;
    TTun(TTun&&) = delete;
    TTun& operator=(const TTun&) = delete;
    TTun& operator=(TTun&&) = delete;
    ~TTun();

    std::error_code Init(const std::string& deviceName);
    std::int32_t GetFd() const;

    std::error_code Write(TBufferView buffer) const noexcept;

    std::expected<TBufferView, std::error_code> Read() noexcept;

private:
    std::int32_t Fd = -1;
    std::size_t MaxBufferSize = 0;
    TBuffer Buffer;
};

using TTunPtr = std::shared_ptr<TTun>;

}
