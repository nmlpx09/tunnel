#include "poll.h"

#include <unistd.h>

namespace NPoll {

TPoll::~TPoll() {
    if (Fd >= 0) {
        close(Fd);
        Fd = -1;
    }
}

TPoll::TPoll(std::int32_t maxPollTimeOut)
: MaxPollTimeOut(maxPollTimeOut) { }

std::error_code TPoll::Init() noexcept {
    if (Fd >= 0) {
        return {};
    }

    if (MaxPollEvents == 0) {
        return EErrorCode::EpollZeroInit;
    }

    if (MaxPollTimeOut > 10000) {
        return EErrorCode::EpollWaitTime;
    }

    if (Fd = epoll_create1(0); Fd < 0) {
        return EErrorCode::EpollCreate;
    }

    Events = std::vector<epoll_event>(MaxPollEvents);

    return {};
}

std::error_code TPoll::RegisterFd(std::int32_t fd) noexcept {
    if (Fd < 0) {
        return EErrorCode::EpollInit;
    }

    auto event = epoll_event {
        .events = EPOLLIN,
        .data = {
            .fd = fd
        }
    };
    if (auto ret = epoll_ctl(Fd, EPOLL_CTL_ADD, fd, &event); ret < 0) {
        return EErrorCode::EpollAdd;
    }

    PollFds.insert(fd);

    return {};
}

std::expected<bool, std::error_code> TPoll::RunOne() noexcept {
    if (Fd < 0) {
        return std::unexpected(EErrorCode::EpollInit);
    }

    const auto numberFd = epoll_wait(Fd, Events.data(), MaxPollEvents, MaxPollTimeOut);

    for (auto index = 0; index < numberFd; ++index) {
        const auto fd = Events[index].data.fd;
        const auto events = Events[index].events;

        if (events & (EPOLLERR | EPOLLHUP)) {
            return std::unexpected(EErrorCode::EpollExit);
        }

        if (PollFds.contains(fd) && (events & EPOLLIN) == events) {
            return true;
        }
    }

    return false;
}

}
