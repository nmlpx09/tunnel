#pragma once

#include <openssl/evp.h>

#include <types.h>

#include <cstdint>
#include <memory>
#include <string>
#include <system_error>

namespace NCrypt {

struct TCrypt {
public:
    TCrypt(std::size_t maxBufferSize);
    TCrypt(const TCrypt&) = delete;
    TCrypt(TCrypt&&) = delete;
    TCrypt& operator=(const TCrypt&) = delete;
    TCrypt& operator=(TCrypt&&) = delete;
    ~TCrypt();

    std::error_code Init(const std::string& key) noexcept;

    TBufferView Encrypt(TBufferView buffer) noexcept;

    TBufferView Decrypt(TBufferView buffer) noexcept;

private:
    static constexpr std::size_t GCM_IV_SIZE = 12;
    static constexpr std::size_t GCM_TAG_SIZE = 16;
    static constexpr std::size_t AES_KEY_SIZE = 16;
    static constexpr std::size_t GCM_IV_TAG_SIZE = GCM_IV_SIZE + GCM_TAG_SIZE;

    using TCipherCtxPtr = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;
    using TDecodeCtxPtr = std::unique_ptr<EVP_ENCODE_CTX, decltype(&EVP_ENCODE_CTX_free)>;

    TCipherCtxPtr EncCtx = TCipherCtxPtr{EVP_CIPHER_CTX_new(), &EVP_CIPHER_CTX_free};
    TCipherCtxPtr DecCtx = TCipherCtxPtr{EVP_CIPHER_CTX_new(), &EVP_CIPHER_CTX_free};

    TDecodeCtxPtr DecodeCtx = TDecodeCtxPtr{EVP_ENCODE_CTX_new(), &EVP_ENCODE_CTX_free};

    TBuffer EncBuffer;
    TBuffer DecBuffer;
};

using TCryptPtr = std::shared_ptr<TCrypt>;

}
