#pragma once

#include <sys/epoll.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace NEpoll {

struct TEpoll {
private:
    using TEvents = std::vector<epoll_event>;
public:
    TEpoll(std::size_t maxEvents) noexcept;
    TEpoll(const TEpoll&) = delete;
    TEpoll(TEpoll&&) = delete;
    TEpoll& operator=(const TEpoll&) = delete;
    TEpoll& operator=(TEpoll&&) = delete;
    ~TEpoll();

    std::int32_t Init() noexcept;

    template <class TFdProviderPtr>
    std::int32_t Add(TFdProviderPtr fdProvider) const noexcept {
        auto event = epoll_event {
            .events = EPOLLIN,
            .data = {
                .fd = fdProvider->Fd
            }
        };
        if (auto ret = epoll_ctl(Fd, EPOLL_CTL_ADD, fdProvider->Fd, &event); ret < 0) {
            return -1;
        }
        return 0;
    }

    std::size_t Wait() noexcept;

    const TEvents& GetEvents() const noexcept;

private:
    std::int32_t Fd = -1;
    std::size_t MaxEvents = 0;
    TEvents Events;
};

using TEpollPtr = std::shared_ptr<TEpoll>;

}
