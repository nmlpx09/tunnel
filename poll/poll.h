#pragma once

#include <errors.h>

#include <sys/epoll.h>

#include <cstdint>
#include <expected>
#include <memory>
#include <system_error>

namespace NPoll {

struct TPoll {
public:
    TPoll(std::int32_t maxPollTimeOut);
    TPoll(const TPoll&) = delete;
    TPoll(TPoll&&) = delete;
    TPoll& operator=(const TPoll&) = delete;
    TPoll& operator=(TPoll&&) = delete;
    ~TPoll();

    std::error_code Init();

    std::error_code RegisterFd(std::int32_t fd) noexcept;

    std::expected<bool, std::error_code> Wait() noexcept;

private:
    std::int32_t Fd = -1;
    std::int32_t MaxPollTimeOut = -1;
    epoll_event Event;
    std::int32_t RegFds = -1;
};

using TPollPtr = std::shared_ptr<TPoll>;

}
