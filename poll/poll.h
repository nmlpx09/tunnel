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
    TPoll(std::int32_t maxPollTimeOut);
    TPoll(const TPoll&) = delete;
    TPoll(TPoll&&) = delete;
    TPoll& operator=(const TPoll&) = delete;
    TPoll& operator=(TPoll&&) = delete;
    ~TPoll();

    std::error_code Init() noexcept;

    template <class TFdProviderPtr>
    std::error_code RegisterFd(TFdProviderPtr fdProvider) noexcept {
        const auto fd = fdProvider->GetFd();

        auto event = epoll_event {
            .events = EPOLLIN,
            .data = {
                .fd = fd
            }
        };
        if (auto ret = epoll_ctl(Fd, EPOLL_CTL_ADD, fd, &event); ret < 0) {
            return EErrorCode::EpollAdd;
        }

        Handlers.insert(fd);

        return {};
    }

    bool RunOne() noexcept;

private:
    std::int32_t Fd = -1;
    std::size_t MaxPollEvents = 1;
    std::int32_t MaxPollTimeOut = -1;
    std::vector<epoll_event> Events;
    std::unordered_set<std::int32_t> Handlers;
};

using TPollPtr = std::shared_ptr<TPoll>;

}
