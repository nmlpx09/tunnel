#include "ips_storage.h"

namespace NIpsStorage {

TElement::TElement(std::string ip, std::uint16_t port)
: Ip(std::move(ip))
, Port(port) {}

void TIpsStorage::Add(std::uint32_t key, std::string ip, std::uint16_t port) noexcept {
    std::unique_lock<std::mutex> ulock{Mutex};
    auto now = std::chrono::steady_clock::now();
    if (now - LiveTime > std::chrono::seconds(3600)) {
        LiveTime = now;
        Map.clear();
    }
    Map[key] = TElement{std::move(ip), port};
}

std::optional<std::pair<std::string, std::uint16_t>> TIpsStorage::Get(std::uint32_t key) noexcept {
    std::unique_lock<std::mutex> ulock{Mutex};
    if (Map.count(key) == 0) {
        return {};
    }
    return std::make_pair(Map[key].Ip, Map[key].Port);
}

}
