#pragma once

#include <crypt/interface.h>
#include <types.h>

#include <openssl/evp.h>

#include <cstdint>
#include <memory>
#include <string>
#include <system_error>

namespace NCrypt {

struct TAes : TInterface {
public:
    TAes() = default;
    TAes(const TAes&) = delete;
    TAes(TAes&&) = delete;
    TAes& operator=(const TAes&) = delete;
    TAes& operator=(TAes&&) = delete;
    ~TAes();

    std::error_code Init(const std::string& key) override;

    TBufferView Encrypt(TBufferView buffer) noexcept override;

    TBufferView Decrypt(TBufferView buffer) noexcept override;

private:
    using TCipherCtxPtr = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;
    using TDecodeCtxPtr = std::unique_ptr<EVP_ENCODE_CTX, decltype(&EVP_ENCODE_CTX_free)>;

    TCipherCtxPtr EncCtx = TCipherCtxPtr{EVP_CIPHER_CTX_new(), &EVP_CIPHER_CTX_free};
    TCipherCtxPtr DecCtx = TCipherCtxPtr{EVP_CIPHER_CTX_new(), &EVP_CIPHER_CTX_free};

    TDecodeCtxPtr DecodeCtx = TDecodeCtxPtr{EVP_ENCODE_CTX_new(), &EVP_ENCODE_CTX_free};

    TBuffer EncBuffer;
    TBuffer DecBuffer;
};

}
