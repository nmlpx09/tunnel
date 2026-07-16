#pragma once

#include <types.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <system_error>

namespace NTun {

struct TTun {
public:
    TTun(std::size_t maxBytesSize);
    TTun(const TTun&) = delete;
    TTun(TTun&&) = delete;
    TTun& operator=(const TTun&) = delete;
    TTun& operator=(TTun&&) = delete;
    ~TTun();

    std::error_code Init(const std::string& deviceName) noexcept;
    std::int32_t GetFd() const noexcept;

    void Write(TBuffer buffer) const noexcept;

    TBuffer Read() noexcept;

private:
    std::int32_t Fd = -1;
    std::size_t MaxBytesSize = 0;
    TBytes Bytes;
};

using TTunPtr = std::shared_ptr<TTun>;

}
