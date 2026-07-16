#include "configs.h"
#include "utils.h"

#include <errors.h>

#include <arpa/inet.h>

#include <fstream>
#include <cstring>
#include <limits>

namespace NUtils {

bool ValidTunFrame(TBuffer buffer) noexcept {
    if (buffer.size() < 24) {
        return false;
    }

    return buffer[0] == 0
        && buffer[1] == 0
        && buffer[2] == 0x08
        && buffer[3] == 0;
}

std::uint32_t GetSrcIpFromTunFrame(TBuffer buffer) noexcept {
    if (buffer.size() < 20) {
        return 0;
    }

    std::uint32_t val;
    std::memcpy(&val, buffer.data() + 16, sizeof(val));
    return val;
}

std::uint32_t GetDstIpFromTunFrame(TBuffer buffer) noexcept {
    if (buffer.size() < 24) {
        return 0;
    }

    std::uint32_t val;
    std::memcpy(&val, buffer.data() + 20, sizeof(val));
    return val;
}

std::pair<std::error_code, TConf> GetConf(bool isClient) noexcept {
    TConf conf;

    if (auto* value = std::getenv("TUN_DEVICE"); value != nullptr) {
        conf.TunDevice = value;
    } else {
        return {EErrorCode::TunDevice, {}};
    }

    if (auto* value = std::getenv("TUN_MTU"); value != nullptr) {
        std::size_t tunMtu = 0;
        try {
            tunMtu = std::stoull(value);
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

    if (auto* value = std::getenv("KEYS_FILE"); value != nullptr) {
        conf.KeysFile = value;
    } else {
        return {EErrorCode::KeysFile, {}};
    }

    if (isClient) {
        if (auto* value = std::getenv("REMOTE_IP"); value != nullptr) {
            std::uint32_t ip;
            if (auto ret = inet_pton(AF_INET, value, &ip); ret == 0) {
                return {EErrorCode::RemoteIp, {}};
            }
            conf.RemoteIp = ip;
        } else {
            return {EErrorCode::RemoteIp, {}};
        }

        if (auto* value = std::getenv("REMOTE_PORT"); value != nullptr) {
            try {
                if (const auto port = std::stoul(value); port > std::numeric_limits<std::uint16_t>::max()) {
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
        if (auto* value = std::getenv("LOCAL_PORT"); value != nullptr) {
            try {
                if (const auto port = std::stoul(value); port > std::numeric_limits<std::uint16_t>::max()) {
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
