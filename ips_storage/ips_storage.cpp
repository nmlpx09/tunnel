#include "ips_storage.h"

namespace NIpsStorage {

TElement::TElement(std::uint32_t ip, std::uint16_t port)
: Ip(ip)
, Port(port) {}

void TIpsStorage::Add(std::uint32_t key, std::uint32_t ip, std::uint16_t port) noexcept {
    if (key == 0) {
        return;
    }
    Map[key].store(TElement{ip, port});
}

std::optional<std::pair<std::uint32_t, std::uint16_t>> TIpsStorage::Get(std::uint32_t key) noexcept {
    auto it = Map.find(key);

    if (it == Map.end()) {
        return {};
    }

    auto value = it->second.load();
    return std::make_pair(value.Ip, value.Port);
}

}
