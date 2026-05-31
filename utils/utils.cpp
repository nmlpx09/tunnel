#include "configs.h"
#include "utils.h"

#include <errors.h>

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

std::pair<std::error_code, TConf> getConf(bool isClient) {
    const auto env = NUtils::getEnv();
    TConf conf;

    if (env.count("tunDevice") > 0) {
        conf.TunDevice = env.at("tunDevice");
    } else {
        return {EErrorCode::TunDevice, {}};
    }

    if (env.count("tunMtu") > 0) {
        std::size_t tunMtu = 0;
        try {
            tunMtu = std::stoi(env.at("tunMtu"));
        } catch (const std::exception&) {
            return {EErrorCode::TunMtuConvert, {}};
        }
        if (tunMtu > MAX_TUN_MTU_SIZE) {
            return {EErrorCode::TunMtuMaxSize, {}};
        }
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
