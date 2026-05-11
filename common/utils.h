#pragma once

#include "types.h"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace NUtils {

bool validIpDatagram(const TBuffer& buffer, std::size_t size);

std::uint32_t getSrcIp(const TBuffer& buffer);

std::uint32_t getDstIp(const TBuffer& buffer);

std::unordered_map<std::string, std::string> getEnv();

}
