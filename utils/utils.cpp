#include "configs.h"
#include "utils.h"

#include <errors.h>

#include <arpa/inet.h>

#include <fstream>
#include <cstring>
#include <limits>

namespace NUtils {

bool ValidTunFrame(const TBuffer& buffer, std::size_t size) noexcept {
    if (size < 24) {
        return false;
    }

    return buffer[0] == 0
        && buffer[1] == 0
        && buffer[2] == 0x08
        && buffer[3] == 0;
}

std::uint32_t GetSrcIpFromTunFrame(const TBuffer& buffer, std::size_t size) noexcept {
    if (size < 20) {
        return 0;
    }

    std::uint32_t val;
    std::memcpy(&val, buffer.data() + 16, sizeof(val));
    return val;
}

std::uint32_t GetDstIpFromTunFrame(const TBuffer& buffer, std::size_t size) noexcept {
    if (size < 24) {
        return 0;
    }

    std::uint32_t val;
    std::memcpy(&val, buffer.data() + 20, sizeof(val));
    return val;
}

std::pair<std::error_code, TConf> GetConf(bool isClient) noexcept {
    TConf conf;

    if (std::getenv("TUN_DEVICE") != nullptr) {
        conf.TunDevice = std::getenv("TUN_DEVICE");
    } else {
        return {EErrorCode::TunDevice, {}};
    }

    if (std::getenv("TUN_MTU") != nullptr) {
        std::size_t tunMtu = 0;
        try {
            tunMtu = std::stoull(std::getenv("TUN_MTU"));
        } catch (const std::exception&) {
            return {EErrorCode::TunMtuConvert, {}};
        }
        if (tunMtu > MAX_TUN_MTU_SIZE || tunMtu == 0) {
            return {EErrorCode::TunMtuMaxSize, {}};
        }
        conf.TunMtu = tunMtu;
    } else {
        return {EErrorCode::TunMtu, {}};
    }

    if (std::getenv("KEYS_FILE") != nullptr) {
        conf.KeysFile = std::getenv("KEYS_FILE");
    } else {
        return {EErrorCode::KeysFile, {}};
    }

    if (isClient) {
        if (std::getenv("REMOTE_IP") != nullptr) {
            std::uint32_t ip;
            if (auto ret = inet_pton(AF_INET, std::getenv("REMOTE_IP"), &ip); ret == 0) {
                return {EErrorCode::RemoteIp, {}};
            }
            conf.RemoteIp = ip;
        } else {
            return {EErrorCode::RemoteIp, {}};
        }

        if (std::getenv("REMOTE_PORT") != nullptr) {
            try {
                if (const auto port = std::stoul(std::getenv("REMOTE_PORT")); port > std::numeric_limits<std::uint16_t>::max()) {
                     return {EErrorCode::RemotePortConvert, {}};
                } else {
                     conf.RemotePort = port;
                }
            } catch (const std::exception&) {
                return {EErrorCode::RemotePortConvert, {}};
            }
        } else {
            return {EErrorCode::RemotePort, {}};
        }
    } else {
        if (std::getenv("LOCAL_PORT") != nullptr) {
            try {
                if (const auto port = std::stoul(std::getenv("LOCAL_PORT")); port > std::numeric_limits<std::uint16_t>::max()) {
                     return {EErrorCode::LocalPortConvert, {}};
                } else {
                     conf.LocalPort = port;
                }
            } catch (const std::exception&) {
                return {EErrorCode::LocalPortConvert, {}};
            }
        } else {
            return {EErrorCode::LocalPort, {}};
        }
    }

    return {{}, std::move(conf)};
}

std::optional<std::pair<std::string, std::string>> LoadKeyPair(const std::string& keysFile) noexcept {
    std::ifstream file(keysFile);
    std::string cipher;
    std::string iv;
    if (!file.is_open() || !std::getline(file, cipher) || !std::getline(file, iv)) {
        return {};
    }
    return std::make_pair(cipher, iv);
}

}
