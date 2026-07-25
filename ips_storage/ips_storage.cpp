#include "ips_storage.h"

namespace NIpsStorage {

void TIpsStorage::Add(std::uint32_t key, std::uint32_t ip, std::uint16_t port) noexcept {
    if (key == 0) {
        return;
    }

    auto newValue = TElement{ip, port};

    if (auto it = Map.find(key); it == Map.end() || it->second != newValue) {
        Map[key] = newValue;
    }
}

std::optional<std::pair<std::uint32_t, std::uint16_t>> TIpsStorage::Get(std::uint32_t key) noexcept {
    if (auto it = Map.find(key); it != Map.end()) {
        auto value = it->second;
        return std::make_pair(value.Ip, value.Port);
    }
    return {};
}

}
