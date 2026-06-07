#pragma once

#include "types.h"

#include <cstdint>
#include <system_error>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

namespace NUtils {

bool ValidIpDatagram(const TBuffer& buffer, std::size_t size) noexcept;

std::uint32_t GetSrcIp(const TBuffer& buffer) noexcept;

std::uint32_t GetDstIp(const TBuffer& buffer) noexcept;

std::unordered_map<std::string, std::string> GetEnv() noexcept;

std::optional<std::pair<std::string, std::string>> LoadKeyPair(const std::string& keysFile) noexcept;

std::pair<std::error_code, TConf> GetConf(bool isClient = true) noexcept;

}
