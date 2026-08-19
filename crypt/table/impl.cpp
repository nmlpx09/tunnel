#include "impl.h"
#include "table.h"

namespace NCrypt {

std::error_code TTable::Init(const std::string&) {
    return {};
}

TBufferView TTable::Encrypt(TBufferView buffer) noexcept {
    if (buffer.size() > EncBuffer.size()) {
        return {};
    }

    std::size_t index = 0;
    for (; index < buffer.size(); ++index) {
        EncBuffer[index] = ENCRYPT_TABLE[buffer[index]];
    }
    return {EncBuffer.begin(), index};
}

TBufferView TTable::Decrypt(TBufferView buffer) noexcept {
    if (buffer.size() > DecBuffer.size()) {
        return {};
    }

    std::size_t index = 0;
    for (; index < buffer.size(); ++index) {
        DecBuffer[index] = DECRYPT_TABLE[buffer[index]];
    }
    return {DecBuffer.begin(), index};
}

}
