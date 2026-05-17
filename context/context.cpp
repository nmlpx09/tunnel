#include "context.h"

namespace NContext {

void TContext::TunWait() noexcept {
    std::unique_lock<std::mutex> ulock{TunMutex};
    TunCv.wait(ulock, [this] { return TunReady > 0 || StopFlag; });
    --TunReady;
}

void TContext::SocketWait() noexcept {
    std::unique_lock<std::mutex> ulock{SocketMutex};
    SocketCv.wait(ulock, [this] { return SocketReady > 0 || StopFlag; });
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

void TContext::TunReset() noexcept {
    std::unique_lock<std::mutex> ulock{TunMutex};
    TunReady = 0;
}

void TContext::SocketReset() noexcept {
    std::unique_lock<std::mutex> ulock{SocketMutex};
    SocketReady = 0;
}

void TContext::Stop() noexcept {
    std::unique_lock<std::mutex> ulock{TunMutex};
    StopFlag = true;
    TunCv.notify_one();
    SocketCv.notify_one();
}

bool TContext::IsStop() noexcept {
    std::unique_lock<std::mutex> ulock{TunMutex};
    return StopFlag;
}

}
