#pragma once

#include "types.h"

#include <cstdint>
#include <system_error>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

namespace NUtils {

bool ValidTunFrame(const TBuffer& buffer, std::size_t size) noexcept;

std::uint32_t GetSrcIpFromTunFrame(const TBuffer& buffer, std::size_t size) noexcept;

std::uint32_t GetDstIpFromTunFrame(const TBuffer& buffer, std::size_t size) noexcept;

std::unordered_map<std::string, std::string> GetEnv() noexcept;

std::optional<std::pair<std::string, std::string>> LoadKeyPair(const std::string& keysFile) noexcept;

std::pair<std::error_code, TConf> GetConf(bool isClient = true) noexcept;

}
