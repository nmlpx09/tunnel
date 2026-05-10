#include "utils.h"

namespace NUtils {

bool validIpDatagram(const TBuffer& buffer, std::size_t size) {
    if (size < 24) {
        return false;
    }
    return *reinterpret_cast<const std::uint32_t*>(buffer.data()) == 0x80000;
}

std::uint32_t getSrcIp(const TBuffer& buffer) {
    return *reinterpret_cast<const std::uint32_t*>(buffer.data() + 16);
}

std::uint32_t getDstIp(const TBuffer& buffer) {
    return *reinterpret_cast<const std::uint32_t*>(buffer.data() + 20);
}

}
