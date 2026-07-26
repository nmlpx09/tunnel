#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>

namespace NIpsStorage {

using TTimestamp = std::uint64_t;

struct TElement {
    std::uint32_t Ip = 0;
    std::uint16_t Port = 0;
    TTimestamp Ts = 0;
};

using TMap = std::unordered_map<std::uint32_t, TElement>;
using TStorage = std::pair<TMap, TTimestamp>;

struct TIpsStorage {
public:
    TIpsStorage();
    TIpsStorage(const TIpsStorage&) = delete;
    TIpsStorage(TIpsStorage&&) = delete;
    TIpsStorage& operator=(const TIpsStorage&) = delete;
    TIpsStorage& operator=(TIpsStorage&&) = delete;
    ~TIpsStorage() = default;

    void Write(std::uint32_t key, std::uint32_t ip, std::uint16_t port) noexcept;
    std::optional<std::pair<std::uint32_t, std::uint16_t>> Read(std::uint32_t key) noexcept;
private:
    std::atomic<std::shared_ptr<TStorage>> ReadStorage;
    std::shared_ptr<TStorage> WriteStorage;
    const TTimestamp RemoveDelay = 3600;
};

using TIpsStoragePtr = std::shared_ptr<TIpsStorage>;

}
