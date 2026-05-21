#pragma once

#include <errors.h>

#include <sys/epoll.h>

#include <cstdint>
#include <memory>
#include <system_error>
#include <vector>
#include <unordered_set>

namespace NPoll {

struct TPoll {
public:
    TPoll(std::size_t maxEvents) noexcept;
    TPoll(const TPoll&) = delete;
    TPoll(TPoll&&) = delete;
    TPoll& operator=(const TPoll&) = delete;
    TPoll& operator=(TPoll&&) = delete;
    ~TPoll();

    std::error_code Init() noexcept;

    template <class TFdProviderPtr>
    std::error_code Add(TFdProviderPtr fdProvider) const noexcept {
        auto event = epoll_event {
            .events = EPOLLIN,
            .data = {
                .fd = fdProvider->Fd
            }
        };
        if (auto ret = epoll_ctl(Fd, EPOLL_CTL_ADD, fdProvider->Fd, &event); ret < 0) {
            return EErrorCode::EpollAdd;
        }
        return {};
    }

    template <class TFdProviderPtr>
    bool Contains(TFdProviderPtr fdProvider) const noexcept {
        return FdSet.contains(fdProvider->Fd);
    }

    void Wait() noexcept;

private:
    std::int32_t Fd = -1;
    std::size_t MaxEvents = 0;
    std::vector<epoll_event> Events;
    std::unordered_set<std::int32_t> FdSet;
};

using TPollPtr = std::shared_ptr<TPoll>;

}
