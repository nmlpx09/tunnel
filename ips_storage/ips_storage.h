#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

namespace NIpsStorage {

struct TElement {
public:
    TElement(std::string ip, std::uint16_t port);
    TElement() = default;
private:
    friend class TIpsStorage;
    std::string Ip;
    std::uint16_t Port = 0;
};

struct TIpsStorage {
public:
    TIpsStorage() noexcept = default;
    TIpsStorage(const TIpsStorage&) = delete;
    TIpsStorage(TIpsStorage&&) = delete;
    TIpsStorage& operator=(const TIpsStorage&) = delete;
    TIpsStorage& operator=(TIpsStorage&&) = delete;
    ~TIpsStorage() = default;

    void Add(std::uint32_t key, const std::string& ip, std::uint16_t port) noexcept;
    std::optional<std::pair<std::string, std::uint16_t>> Get(std::uint32_t key) noexcept;

private:
    std::unordered_map<std::uint32_t, TElement> Map;
    std::chrono::steady_clock::time_point LiveTime = std::chrono::steady_clock::now();
    std::mutex Mutex;
};

using TIpsStoragePtr = std::shared_ptr<TIpsStorage>;

}
