#pragma once

#include <cstdint>
#include <cstddef>

static constexpr std::size_t MAX_POLL_EVENTS = 1;
static constexpr std::int32_t MAX_POLL_TIMEOUT_MS = 1000;
static constexpr std::size_t MAX_DATA_SIZE = 1500;
static constexpr std::size_t MAX_TUN_MTU_SIZE = MAX_DATA_SIZE - 40;
