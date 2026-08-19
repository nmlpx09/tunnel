#pragma once

#include <cstdint>
#include <cstddef>

inline constexpr std::int32_t MAX_POLL_TIMEOUT_MS = 1000;
inline constexpr std::size_t MAX_DATA_SIZE = 1500;
inline constexpr std::size_t IP_HEADER_SIZE = 20;
inline constexpr std::size_t UDP_HEADER_SIZE = 8;
inline constexpr std::size_t GCM_IV_SIZE = 12;
inline constexpr std::size_t GCM_TAG_SIZE = 16;
inline constexpr std::size_t GCM_KEY_SIZE = 16;
inline constexpr std::size_t GCM_OVERHEAD = GCM_IV_SIZE + GCM_TAG_SIZE;
inline constexpr std::size_t PROTOCOL_OVERHEAD = IP_HEADER_SIZE + UDP_HEADER_SIZE;
#ifdef TABLE
inline constexpr std::size_t MAX_TUN_MTU_SIZE = MAX_DATA_SIZE - PROTOCOL_OVERHEAD;
#else
inline constexpr std::size_t MAX_TUN_MTU_SIZE = MAX_DATA_SIZE - PROTOCOL_OVERHEAD - GCM_OVERHEAD;
#endif

inline constexpr std::size_t SOCKET_BUFFER_SIZE = 4 * 1024 * 1024;
