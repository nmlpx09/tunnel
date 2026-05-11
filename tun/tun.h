#pragma once

#include <common/types.h>
#include <epoll/epoll.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <tuple>

namespace NTun {

struct TTun {
public:
    TTun(std::size_t maxBufferSize) noexcept;
    TTun(const TTun&) = delete;
    TTun(TTun&&) = delete;
    TTun& operator=(const TTun&) = delete;
    TTun& operator=(TTun&&) = delete;
    ~TTun();

    std::int32_t Init(std::string deviceName) noexcept;

    void Write(const TBuffer& buffer, std::size_t size) const noexcept;

    std::tuple<
        std::size_t,
        std::reference_wrapper<const TBuffer>
    > Read() noexcept;

    bool IsFd(std::int32_t fd) const noexcept;

private:
    std::int32_t Fd = -1;
    std::size_t MaxBufferSize = 0;
    TBuffer Buffer;
    friend class NEpoll::TEpoll;
};

using TTunPtr = std::shared_ptr<TTun>;

}
