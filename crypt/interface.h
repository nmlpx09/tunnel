#pragma once

#include <types.h>

#include <memory>
#include <string>

namespace NCrypt {

struct TInterface {
    virtual std::error_code Init(const std::string&) = 0;

    virtual TBufferView Encrypt(TBufferView) noexcept = 0;

    virtual TBufferView Decrypt(TBufferView) noexcept = 0;

    virtual ~TInterface() = default;
};

using TCryptPtr = std::shared_ptr<TInterface>;

}
