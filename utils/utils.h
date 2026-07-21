#pragma once

#include "types.h"

#include <cstdint>
#include <system_error>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

namespace NUtils {

bool ValidTunFrame(TBufferView buffer) noexcept;

std::uint32_t GetSrcIpFromTunFrame(TBufferView buffer) noexcept;

std::uint32_t GetDstIpFromTunFrame(TBufferView buffer) noexcept;

std::optional<std::string> LoadKey(const std::string& keysFile) noexcept;

std::pair<std::error_code, TConf> GetConf(bool isClient = true) noexcept;

}
