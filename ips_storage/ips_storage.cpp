#include "ips_storage.h"

namespace NIpsStorage {

TIpsStorage::TIpsStorage()
: ReadStorage(std::make_shared<TStorage>(TMap{}, 0))
, WriteStorage(std::make_shared<TStorage>(TMap{}, 0)) {}

void TIpsStorage::Write(std::uint32_t key, std::uint32_t ip, std::uint16_t port) noexcept {
    if (key == 0) {
        return;
    }

    const std::uint64_t now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();

    if (now - WriteStorage->second > RemoveDelay) {
        WriteStorage->second = now;
        std::erase_if(WriteStorage->first,
            [now, removeDelay = RemoveDelay](const auto& v) { return now - v.second.Ts > removeDelay; }
        );
    }

    WriteStorage->first[key] = TElement{ip, port, now};
    WriteStorage = ReadStorage.exchange(WriteStorage, std::memory_order_release);
}

std::optional<std::pair<std::uint32_t, std::uint16_t>> TIpsStorage::Read(std::uint32_t key) noexcept {
    const auto storage = ReadStorage.load(std::memory_order_acquire);
    if (const auto it = storage->first.find(key); it != storage->first.end()) {
        const auto value = it->second;
        return std::make_pair(value.Ip, value.Port);
    }
    return {};
}

}
