#include "ips_storage.h"

namespace NIpsStorage {

TElement::TElement(std::string ip, std::uint16_t port)
    : Ip(std::move(ip))
    , Port(port) {}

bool TElement::operator ==(const TElement& lhs) const {
    return Ip == lhs.Ip && Port == lhs.Port;
}

void TIpsStorage::Add(std::uint32_t key, std::string ip, std::uint16_t port) noexcept {
    std::unique_lock<std::mutex> ulock{Mutex};
    const auto newElement = TElement{std::move(ip), port};
    if (Map.count(key) == 0 || (Map[key] != newElement && std::chrono::steady_clock::now() - Map[key].Timestamp > std::chrono::seconds(10))) {
        Map[key] = newElement;
    }
}

std::optional<std::pair<std::string, std::uint16_t>> TIpsStorage::Get(std::uint32_t key) noexcept {
    std::unique_lock<std::mutex> ulock{Mutex};
    if (Map.count(key) == 0) {
        return {};
    }
    return std::make_pair(Map[key].Ip, Map[key].Port);
}

}
