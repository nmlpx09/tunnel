#include "utils.h"

namespace NUtils {

bool validIpDatagram(const TBuffer& buffer, std::size_t size) {
    if (size < 20) {
        return false;
    }
    return *reinterpret_cast<const std::uint32_t*>(buffer.data()) == 0x80000;
}

}
