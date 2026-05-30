#include "utils.h"

#include <fstream>
#include <cstdlib>

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

std::unordered_map<std::string, std::string> getEnv() {
    std::unordered_map<std::string, std::string> result;
    if (std::getenv("TUN_DEVICE") != nullptr) {
        result.insert({"tunDevice", std::getenv("TUN_DEVICE")});
    }
    if (std::getenv("REMOTE_IP") != nullptr) {
        result.insert({"remoteIp", std::getenv("REMOTE_IP")});
    }
    if (std::getenv("REMOTE_PORT") != nullptr) {
        result.insert({"remotePort", std::getenv("REMOTE_PORT")});
    }
    if (std::getenv("LOCAL_PORT") != nullptr) {
        result.insert({"localPort", std::getenv("LOCAL_PORT")});
    }
    if (std::getenv("TUN_MTU") != nullptr) {
        result.insert({"tunMtu", std::getenv("TUN_MTU")});
    }
    if (std::getenv("KEYS_FILE") != nullptr) {
        result.insert({"keysFile", std::getenv("KEYS_FILE")});
    }
    return result;
}

std::optional<std::pair<std::string, std::string>> loadKeyPair(const std::string& keysFile) {
    std::ifstream file(keysFile);
    std::string chiper;
    std::string iv;
    if (!file.is_open() || !std::getline(file, chiper) || !std::getline(file, iv)) {
        return {};
    }
    return std::make_pair(chiper, iv);
}

}
