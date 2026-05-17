#include "epoll.h"

#include <unistd.h>

namespace NEpoll {

TEpoll::~TEpoll() {
    close(Fd);
    Fd = -1;
}

TEpoll::TEpoll(std::size_t maxEvents) noexcept : MaxEvents(maxEvents) { }

std::error_code TEpoll::Init() noexcept {
    if (MaxEvents == 0) {
        return EErrorCode::EpollZeroInit;
    }

    if (Fd = epoll_create1(0); Fd < 0) {
        return EErrorCode::EpollCreate;
    }

    Events = std::vector<epoll_event>(MaxEvents);
    return {};
}

std::int32_t TEpoll::Wait() noexcept {
    if (Fd < 0) {
        return 0;
    }
    return epoll_wait(Fd, Events.data(), MaxEvents, -1);
}

const TEpoll::TEvents& TEpoll::GetEvents() const noexcept {
    return Events;
}

}
