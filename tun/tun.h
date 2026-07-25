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
    TTun(std::size_t maxBufferSize);
    TTun(const TTun&) = delete;
    TTun(TTun&&) = delete;
    TTun& operator=(const TTun&) = delete;
    TTun& operator=(TTun&&) = delete;
    ~TTun();

    std::error_code Init(const std::string& deviceName);
    std::int32_t GetFd() const;

    void Write(TBufferView buffer) const noexcept;

    TBufferView Read() noexcept;

private:
    std::int32_t Fd = -1;
    std::size_t MaxBufferSize = 0;
    TBuffer Buffer;
};

using TTunPtr = std::shared_ptr<TTun>;

}
