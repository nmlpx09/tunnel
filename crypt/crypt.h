#pragma once

#include <openssl/evp.h>

#include <types.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <system_error>
#include <tuple>

namespace NCrypt {

struct TCrypt {
public:
    TCrypt(std::size_t maxBufferSize);
    TCrypt(const TCrypt&) = delete;
    TCrypt(TCrypt&&) = delete;
    TCrypt& operator=(const TCrypt&) = delete;
    TCrypt& operator=(TCrypt&&) = delete;
    ~TCrypt() = default;

    std::error_code Init(const std::string& cipher, const std::string& iv) noexcept;

    std::tuple<
        std::reference_wrapper<const TBuffer>,
        std::size_t
    > Encrypt(const TBuffer& buffer, std::size_t size) noexcept;

    std::tuple<
        std::reference_wrapper<const TBuffer>,
        std::size_t
    > Decrypt(const TBuffer& buffer, std::size_t size) noexcept;

private:
    using TCipherCtxPtr = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;
    TCipherCtxPtr EncCtx = TCipherCtxPtr{EVP_CIPHER_CTX_new(), &EVP_CIPHER_CTX_free};
    TCipherCtxPtr DecCtx = TCipherCtxPtr{EVP_CIPHER_CTX_new(), &EVP_CIPHER_CTX_free};

    TBuffer EncBuffer;
    TBuffer DecBuffer;
};

using TCryptPtr = std::shared_ptr<TCrypt>;

}
