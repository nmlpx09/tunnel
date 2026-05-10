#pragma once

#include "types.h"

#include <cstdint>

namespace NUtils {

bool validIpDatagram(const TBuffer& buffer, std::size_t size);

std::uint32_t getSrcIp(const TBuffer& buffer);

std::uint32_t getDstIp(const TBuffer& buffer);

}
