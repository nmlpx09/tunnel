#include "epoll.h"

namespace NEpoll {

TEpoll::~TEpoll() {
    close(Fd);
    Fd = -1;
}

TEpoll::TEpoll(std::size_t maxEvents) noexcept : MaxEvents(maxEvents) { }

std::int32_t TEpoll::Init() noexcept {
    if (MaxEvents == 0) {
        return -1;
    }

    if (Fd = epoll_create1(0); Fd < 0) {
        return -1;
    }

    Events = std::vector<epoll_event>(MaxEvents);
    return 0;
}

std::size_t TEpoll::Wait() noexcept {
    return epoll_wait(Fd, Events.data(), MaxEvents, -1);
}

const TEpoll::TEvents& TEpoll::GetEvents() const noexcept {
    return Events;
}

}
