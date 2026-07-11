#include "context.h"

namespace NContext {

void TContext::TunWait() noexcept {
    std::unique_lock<std::mutex> ulock{TunMutex};
    TunCv.wait(ulock, [this] { return TunReady > 0; });
    --TunReady;
}

void TContext::SocketWait() noexcept {
    std::unique_lock<std::mutex> ulock{SocketMutex};
    SocketCv.wait(ulock, [this] { return SocketReady > 0; });
    --SocketReady;
}

void TContext::TunNotify() noexcept {
    std::unique_lock<std::mutex> ulock{TunMutex};
    ++TunReady;
    TunCv.notify_one();
}

void TContext::SocketNotify() noexcept {
    std::unique_lock<std::mutex> ulock{SocketMutex};
    ++SocketReady;
    SocketCv.notify_one();
}

}
