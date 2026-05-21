#include "poll.h"

#include <unistd.h>

namespace NPoll {

TPoll::~TPoll() {
    close(Fd);
    Fd = -1;
}

TPoll::TPoll(std::size_t maxEvents) noexcept : MaxEvents(maxEvents) { }

std::error_code TPoll::Init() noexcept {
    if (MaxEvents == 0) {
        return EErrorCode::EpollZeroInit;
    }

    if (Fd = epoll_create1(0); Fd < 0) {
        return EErrorCode::EpollCreate;
    }

    Events = std::vector<epoll_event>(MaxEvents);
    return {};
}

void TPoll::Wait() noexcept {
    FdSet.clear();

    if (Fd < 0) {
        return;
    }
    const auto numberFd = epoll_wait(Fd, Events.data(), MaxEvents, 1000);

    for (auto index = 0; index < numberFd; ++index) {
        FdSet.insert(Events[index].data.fd);
    }

    return;
}

}
