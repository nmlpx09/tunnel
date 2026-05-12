#pragma once

#include <openssl/evp.h>

#include <common/types.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <tuple>

namespace NCrypt {

struct TCrypt {
public:
    TCrypt(std::size_t maxBufferSize) noexcept;
    TCrypt(const TCrypt&) = delete;
    TCrypt(TCrypt&&) = delete;
    TCrypt& operator=(const TCrypt&) = delete;
    TCrypt& operator=(TCrypt&&) = delete;
    ~TCrypt();

    std::int32_t Init(std::string chiper, std::string iv) noexcept;

    std::tuple<
        std::reference_wrapper<const TBuffer>,
        std::size_t
    > Encrypt(const TBuffer& buffer, std::size_t size) noexcept;

    std::tuple<
        std::reference_wrapper<const TBuffer>,
        std::size_t
    > Decrypt(const TBuffer& buffer, std::size_t size) noexcept;

private:
    EVP_CIPHER_CTX* EncCtx = nullptr;
    EVP_CIPHER_CTX* DecCtx = nullptr;
    TBuffer EncBuffer;
    TBuffer DecBuffer;
};

using TCryptPtr = std::shared_ptr<TCrypt>;

}
