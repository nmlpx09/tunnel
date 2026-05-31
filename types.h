#pragma once

#include <cstdint>
#include <vector>
#include <string>

using TBuffer = std::vector<std::uint8_t>;

struct TConf {
    std::string TunDevice;
    std::string RemoteIp;
    std::uint16_t RemotePort = 0;
    std::string LocalIp = "0.0.0.0";
    std::uint16_t LocalPort = 0;
    std::string KeysFile;
};
