#pragma once

#include "types.h"

#include <cstdint>
#include <system_error>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

namespace NUtils {

bool validIpDatagram(const TBuffer& buffer, std::size_t size) noexcept;

std::uint32_t getSrcIp(const TBuffer& buffer) noexcept;

std::uint32_t getDstIp(const TBuffer& buffer) noexcept;

std::unordered_map<std::string, std::string> getEnv() noexcept;

std::optional<std::pair<std::string, std::string>> loadKeyPair(const std::string& keysFile) noexcept;

std::pair<std::error_code, TConf> getConf(bool isClient = true) noexcept;

}
