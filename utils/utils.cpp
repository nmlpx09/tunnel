#include "configs.h"
#include "utils.h"

#include <errors.h>

#include <fstream>
#include <cstring>

namespace NUtils {

bool ValidTunFrame(const TBuffer& buffer, std::size_t size) noexcept {
    if (size < 24) {
        return false;
    }

    std::uint32_t val;
    std::memcpy(&val, buffer.data(), sizeof(val));
    return val == 0x00080000;
}

std::uint32_t GetSrcIpFromTunFrame(const TBuffer& buffer, std::size_t size) noexcept {
    if (size < 24) {
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

std::unordered_map<std::string, std::string> GetEnv() noexcept {
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

std::pair<std::error_code, TConf> GetConf(bool isClient) noexcept {
    const auto env = GetEnv();
    TConf conf;

    if (env.count("tunDevice") > 0) {
        conf.TunDevice = env.at("tunDevice");
    } else {
        return {EErrorCode::TunDevice, {}};
    }

    if (env.count("tunMtu") > 0) {
        std::size_t tunMtu = 0;
        try {
            tunMtu = std::stoull(env.at("tunMtu"));
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

    if (env.count("keysFile") > 0) {
        conf.KeysFile = env.at("keysFile");
    } else {
        return {EErrorCode::KeysFile, {}};
    }

    if (isClient) {
        if (env.count("remoteIp") > 0) {
            conf.RemoteIp = env.at("remoteIp");
        } else {
            return {EErrorCode::RemoteIp, {}};
        }

        if (env.count("remotePort") > 0) {
            try {
                conf.RemotePort = std::stoi(env.at("remotePort"));
            } catch (const std::exception&) {
                return {EErrorCode::RemotePortConvert, {}};
            }
        } else {
            return {EErrorCode::RemotePort, {}};
        }
    } else {
        if (env.count("localPort") > 0) {
            try {
                conf.LocalPort = std::stoi(env.at("localPort"));
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
