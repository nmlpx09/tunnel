#pragma once

#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>

namespace NContext {

struct TContext {
public:
    TContext() noexcept = default;
    TContext(const TContext&) = delete;
    TContext(TContext&&) = delete;
    TContext& operator=(const TContext&) = delete;
    TContext& operator=(TContext&&) = delete;
    ~TContext() = default;

    void TunWait() noexcept;
    void SocketWait() noexcept;
    void TunNotify() noexcept;
    void SocketNotify() noexcept;
private:
    std::mutex TunMutex;
    std::mutex SocketMutex;
    std::condition_variable TunCv;
    std::condition_variable SocketCv;
    std::size_t TunReady = 0;
    std::size_t SocketReady = 0;
};

using TContextPtr = std::shared_ptr<TContext>;

}
