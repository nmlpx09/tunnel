#pragma once

#include "interface.h"

#include <types.h>

#include <string>
#include <system_error>

namespace NCrypt {

struct TTable : TInterface {
public:
    TTable() = default;
    TTable(const TTable&) = delete;
    TTable(TTable&&) = delete;
    TTable& operator=(const TTable&) = delete;
    TTable& operator=(TTable&&) = delete;
    ~TTable() = default;

    std::error_code Init(const std::string& key) override;

    TBufferView Encrypt(TBufferView buffer) noexcept override;

    TBufferView Decrypt(TBufferView buffer) noexcept override;

private:
    TBuffer EncBuffer;
    TBuffer DecBuffer;
};

}
