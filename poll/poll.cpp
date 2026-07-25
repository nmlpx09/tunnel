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

std::error_code TPoll::Init() {
    if (Fd >= 0) {
        return {};
    }

    if (MaxPollTimeOut > 10000) {
        return EErrorCode::EpollWaitTime;
    }

    if (Fd = epoll_create1(0); Fd < 0) {
        return EErrorCode::EpollCreate;
    }

    return {};
}

std::error_code TPoll::RegisterFd(std::int32_t fd) noexcept {
    if (Fd < 0) {
        return EErrorCode::EpollInit;
    }

    if (RegFds >= 0) {
        return {};
    }

    auto event = epoll_event {
        .events = EPOLLIN | EPOLLET,
        .data = {
            .fd = fd
        }
    };
    if (auto ret = epoll_ctl(Fd, EPOLL_CTL_ADD, fd, &event); ret < 0) {
        return EErrorCode::EpollAdd;
    }

    RegFds = fd;

    return {};
}

std::expected<bool, std::error_code> TPoll::Wait() noexcept {
    if (Fd < 0) {
        return std::unexpected(EErrorCode::EpollInit);
    }

    const auto numberFd = epoll_wait(Fd, &Event, 1, MaxPollTimeOut);

    if (numberFd == 0) {
        return false;
    }

    const auto fd = Event.data.fd;
    const auto events = Event.events;

    if (events & (EPOLLERR | EPOLLHUP)) {
        return std::unexpected(EErrorCode::EpollExit);
    }

    if (RegFds == fd && (events & EPOLLIN)) {
        return true;
    }

    return false;
}

}
