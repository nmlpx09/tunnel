#pragma once

#include "types.h"

#include <cstdint>
#include <system_error>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

namespace NUtils {

bool ValidIpv4Packet(TBufferView buffer) noexcept;

std::uint32_t GetSrcIpFromIpv4Packet(TBufferView buffer) noexcept;

std::uint32_t GetDstIpFromIpv4Packet(TBufferView buffer) noexcept;

std::optional<std::string> LoadKey(const std::string& keysFile);

std::pair<std::error_code, TConf> GetConf(bool isClient = true);

}
