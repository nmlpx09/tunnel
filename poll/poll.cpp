#include "poll.h"

#include <unistd.h>

namespace NPoll {

TPoll::~TPoll() {
    close(Fd);
    Fd = -1;
}

TPoll::TPoll(std::size_t maxPollEvents, std::int32_t maxPollTimeOut) noexcept
: MaxPollEvents(maxPollEvents)
, MaxPollTimeOut(maxPollTimeOut) { }

std::error_code TPoll::Init() noexcept {
    if (MaxPollEvents == 0) {
        return EErrorCode::EpollZeroInit;
    }

    if (Fd = epoll_create1(0); Fd < 0) {
        return EErrorCode::EpollCreate;
    }

    Events = std::vector<epoll_event>(MaxPollEvents);
    return {};
}

void TPoll::RunOne() noexcept {
    if (Fd < 0) {
        return;
    }
    const auto numberFd = epoll_wait(Fd, Events.data(), MaxPollEvents, MaxPollTimeOut);

    for (auto index = 0; index < numberFd; ++index) {
        if (auto fd = Events[index].data.fd; Handlers.contains(fd)) {
            std::invoke(Handlers[fd]);
        }
    }

    return;
}

}
