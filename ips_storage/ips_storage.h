#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>

namespace NIpsStorage {

struct TElement {
    std::uint32_t Ip = 0;
    std::uint16_t Port = 0;
};

struct TIpsStorage {
public:
    TIpsStorage();
    TIpsStorage(const TIpsStorage&) = delete;
    TIpsStorage(TIpsStorage&&) = delete;
    TIpsStorage& operator=(const TIpsStorage&) = delete;
    TIpsStorage& operator=(TIpsStorage&&) = delete;
    ~TIpsStorage() = default;

    void Add(std::uint32_t key, std::uint32_t ip, std::uint16_t port) noexcept;
    std::optional<std::pair<std::uint32_t, std::uint16_t>> Get(std::uint32_t key) noexcept;

private:
    using TMap = std::unordered_map<std::uint32_t, TElement>;
    std::atomic<std::shared_ptr<TMap>> GetMap;
    std::shared_ptr<TMap> AddMap;
    std::chrono::steady_clock::time_point LiveTime = std::chrono::steady_clock::now();
};

using TIpsStoragePtr = std::shared_ptr<TIpsStorage>;

}
