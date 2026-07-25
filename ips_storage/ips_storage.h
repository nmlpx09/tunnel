#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

namespace NIpsStorage {

struct TElement {
    std::uint32_t Ip = 0;
    std::uint16_t Port = 0;

    bool operator ==(const TElement& lhs) const = default;
};

struct TIpsStorage {
public:
    TIpsStorage() noexcept = default;
    TIpsStorage(const TIpsStorage&) = delete;
    TIpsStorage(TIpsStorage&&) = delete;
    TIpsStorage& operator=(const TIpsStorage&) = delete;
    TIpsStorage& operator=(TIpsStorage&&) = delete;
    ~TIpsStorage() = default;

    void Add(std::uint32_t key, std::uint32_t ip, std::uint16_t port) noexcept;
    std::optional<std::pair<std::uint32_t, std::uint16_t>> Get(std::uint32_t key) noexcept;

private:
    std::unordered_map<std::uint32_t, TElement> Map;
};

using TIpsStoragePtr = std::shared_ptr<TIpsStorage>;

}
