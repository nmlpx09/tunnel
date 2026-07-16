#pragma once

#include <openssl/evp.h>

#include <types.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <system_error>

namespace NCrypt {

struct TCrypt {
public:
    TCrypt(std::size_t maxBytesSize);
    TCrypt(const TCrypt&) = delete;
    TCrypt(TCrypt&&) = delete;
    TCrypt& operator=(const TCrypt&) = delete;
    TCrypt& operator=(TCrypt&&) = delete;
    ~TCrypt() = default;

    std::error_code Init(const std::string& cipher, const std::string& iv) noexcept;

    TBuffer Encrypt(TBuffer buffer) noexcept;

    TBuffer Decrypt(TBuffer buffer) noexcept;

private:
    using TCipherCtxPtr = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;
    using TDecodeCtxPtr = std::unique_ptr<EVP_ENCODE_CTX, decltype(&EVP_ENCODE_CTX_free)>;

    TCipherCtxPtr EncCtx = TCipherCtxPtr{EVP_CIPHER_CTX_new(), &EVP_CIPHER_CTX_free};
    TCipherCtxPtr DecCtx = TCipherCtxPtr{EVP_CIPHER_CTX_new(), &EVP_CIPHER_CTX_free};

    TDecodeCtxPtr DecodeCtx = TDecodeCtxPtr{EVP_ENCODE_CTX_new(), &EVP_ENCODE_CTX_free};

    TBytes EncBytes;
    TBytes DecBytes;
};

using TCryptPtr = std::shared_ptr<TCrypt>;

}
