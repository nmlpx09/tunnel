#include "impl.h"
#include "table.h"

#include <algorithm>

namespace NCrypt {

std::error_code TTable::Init(const std::string&) {
    return {};
}

TBufferView TTable::Encrypt(TBufferView buffer) noexcept {
    if (buffer.size() > EncBuffer.size()) {
        return {};
    }

    std::transform(buffer.cbegin(), buffer.cend(), EncBuffer.begin(),
        [&](const auto& v) { return ENCRYPT_TABLE[v]; });

    return {EncBuffer.begin(), buffer.size()};
}

TBufferView TTable::Decrypt(TBufferView buffer) noexcept {
    if (buffer.size() > DecBuffer.size()) {
        return {};
    }

    std::transform(buffer.cbegin(), buffer.cend(), DecBuffer.begin(),
        [&](const auto& v) { return DECRYPT_TABLE[v]; });

    return {DecBuffer.begin(), buffer.size()};
}

}
