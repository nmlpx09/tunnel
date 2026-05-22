#pragma once

#include <errors.h>

#include <sys/epoll.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <system_error>
#include <vector>
#include <unordered_map>

namespace NPoll {

struct TPoll {
private:
    using TTask = std::function<void()>;
public:
    TPoll(std::size_t maxPollEvents, std::int32_t maxPollTimeOut) noexcept;
    TPoll(const TPoll&) = delete;
    TPoll(TPoll&&) = delete;
    TPoll& operator=(const TPoll&) = delete;
    TPoll& operator=(TPoll&&) = delete;
    ~TPoll();

    std::error_code Init() noexcept;

    template <class TFdProviderPtr>
    std::error_code RegisterHandlerIn(TFdProviderPtr fdProvider, TTask task) noexcept {
        const auto fd = fdProvider->Fd;

        auto event = epoll_event {
            .events = EPOLLIN,
            .data = {
                .fd = fd
            }
        };
        if (auto ret = epoll_ctl(Fd, EPOLL_CTL_ADD, fd, &event); ret < 0) {
            return EErrorCode::EpollAdd;
        }

        Handlers[fd] = std::move(task);

        return {};
    }

    void RunOne() noexcept;

private:
    std::int32_t Fd = -1;
    std::size_t MaxPollEvents = 0;
    std::int32_t MaxPollTimeOut = -1;
    std::vector<epoll_event> Events;
    std::unordered_map<std::int32_t, TTask> Handlers;
};

using TPollPtr = std::shared_ptr<TPoll>;

}
