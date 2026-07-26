#include "ips_storage.h"

namespace NIpsStorage {

TIpsStorage::TIpsStorage()
: GetMap(std::make_shared<TMap>())
, AddMap(std::make_shared<TMap>()) {}

void TIpsStorage::Add(std::uint32_t key, std::uint32_t ip, std::uint16_t port) noexcept {
    if (key == 0) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    if (now - LiveTime > std::chrono::seconds(3600)) {
        LiveTime = now;
        AddMap->clear();
    }

    (*AddMap)[key] = TElement{ip, port};
    AddMap = GetMap.exchange(AddMap, std::memory_order_release);
}

std::optional<std::pair<std::uint32_t, std::uint16_t>> TIpsStorage::Get(std::uint32_t key) noexcept {
    const auto map = GetMap.load(std::memory_order_acquire);
    if (const auto it = map->find(key); it != map->end()) {
        const auto value = it->second;
        return std::make_pair(value.Ip, value.Port);
    }
    return {};
}

}
