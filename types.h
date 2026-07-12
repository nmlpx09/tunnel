#pragma once

#include <cstdint>
#include <vector>
#include <string>

using TBuffer = std::vector<std::uint8_t>;

struct TConf {
    std::string TunDevice;
    std::size_t TunMtu = 0;
    std::uint32_t RemoteIp = 0;
    std::uint16_t RemotePort = 0;
    std::uint32_t LocalIp = 0;
    std::uint16_t LocalPort = 0;
    std::string KeysFile;
};
