#pragma once

#include <errors.h>

#include <sys/epoll.h>

#include <cstdint>
#include <expected>
#include <memory>
#include <system_error>
#include <vector>
#include <unordered_set>

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

    std::expected<bool, std::error_code> RunOne() noexcept;

private:
    std::int32_t Fd = -1;
    std::size_t MaxPollEvents = 1;
    std::int32_t MaxPollTimeOut = -1;
    std::vector<epoll_event> Events;
    std::unordered_set<std::int32_t> PollFds;
};

using TPollPtr = std::shared_ptr<TPoll>;

}
