#pragma once

#include <cstdint>

static constexpr std::size_t MAX_POLL_EVENTS = 2;
static constexpr std::int32_t MAX_POLL_TIMEOUT_MS = 1000;
static constexpr std::size_t MAX_DATA_SIZE = 1500;
static constexpr std::size_t MAX_MTU_SIZE = MAX_DATA_SIZE - 40;
