#include "crypt.h"

#include <openssl/aes.h>

namespace NCrypt {

TCrypt::TCrypt(std::size_t maxBufferSize) noexcept
: EncBuffer(maxBufferSize + AES_BLOCK_SIZE, 0)
, DecBuffer(maxBufferSize + AES_BLOCK_SIZE, 0) {}

std::int32_t TCrypt::Init(std::string chiper, std::string iv) noexcept {
    if (auto ret = EVP_EncryptInit_ex(EncCtx.get(), EVP_aes_128_cbc(), nullptr,
            reinterpret_cast<const unsigned char*>(chiper.c_str()), reinterpret_cast<const unsigned char*>(iv.c_str())); ret == 0) {
        return -1;
    }

    if (auto ret = EVP_DecryptInit_ex(DecCtx.get(), EVP_aes_128_cbc(), nullptr,
            reinterpret_cast<const unsigned char*>(chiper.c_str()), reinterpret_cast<const unsigned char*>(iv.c_str())); ret == 0) {
        return -1;
    }

    return 0;
}

std::tuple<
    std::reference_wrapper<const TBuffer>,
    std::size_t
> TCrypt::Encrypt(const TBuffer& buffer, std::size_t size) noexcept {
    if (!EncCtx) {
        return {cref(EncBuffer), 0};
    }

    if (size > EncBuffer.size()) {
        return {cref(EncBuffer), 0};
    }

    EVP_EncryptInit_ex(EncCtx.get(), nullptr, nullptr, nullptr, nullptr);

    std::int32_t sizeEnc;
    if (auto ret = EVP_EncryptUpdate(EncCtx.get(), EncBuffer.data(), &sizeEnc, buffer.data(), size); ret == 0) {
        return {cref(EncBuffer), 0};
    }

    std::int32_t sizeEncFinal;
    if (auto ret = EVP_EncryptFinal_ex(EncCtx.get(), EncBuffer.data() + sizeEnc, &sizeEncFinal); ret == 0) {
        return {cref(EncBuffer), 0};
    }

    return {cref(EncBuffer), sizeEnc + sizeEncFinal};
}

std::tuple<
    std::reference_wrapper<const TBuffer>,
    std::size_t
> TCrypt::Decrypt(const TBuffer& buffer, std::size_t size) noexcept {
    if (!DecCtx) {
        return {cref(DecBuffer), 0};
    }

    if (size > DecBuffer.size()) {
        return {cref(DecBuffer), 0};
    }

    EVP_DecryptInit_ex(DecCtx.get(), nullptr, nullptr, nullptr, nullptr);

    std::int32_t sizeDec;
    if (auto ret = EVP_DecryptUpdate(DecCtx.get(), DecBuffer.data(), &sizeDec, buffer.data(), size); ret == 0) {
        return {cref(DecBuffer), 0};
    }

    std::int32_t sizeDecFinal;
    if (auto ret = EVP_DecryptFinal_ex(DecCtx.get(), DecBuffer.data() + sizeDec, &sizeDecFinal); ret == 0) {
        return {cref(DecBuffer), 0};
    }

    return {cref(DecBuffer), sizeDec + sizeDecFinal};
}

}
