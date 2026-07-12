#include "ips_storage.h"

namespace NIpsStorage {

TElement::TElement(std::uint32_t ip, std::uint16_t port)
: Ip(ip)
, Port(port) {}

void TIpsStorage::Add(std::uint32_t key, std::uint32_t ip, std::uint16_t port) noexcept {
    if (key == 0) {
        return;
    }
    std::unique_lock<std::mutex> ulock{Mutex};
    auto now = std::chrono::steady_clock::now();
    if (now - LiveTime > std::chrono::seconds(3600)) {
        LiveTime = now;
        Map.clear();
    }
    Map[key] = TElement{ip, port};
}

std::optional<std::pair<std::uint32_t, std::uint16_t>> TIpsStorage::Get(std::uint32_t key) noexcept {
    std::unique_lock<std::mutex> ulock{Mutex};
    auto it = Map.find(key);

    if (it == Map.end()) {
        return {};
    }
    return std::make_pair(it->second.Ip, it->second.Port);
}

}
