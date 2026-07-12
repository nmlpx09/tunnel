#include "crypt.h"

#include <errors.h>

#include <openssl/aes.h>
#include <openssl/evp.h>

namespace NCrypt {

TCrypt::TCrypt(std::size_t maxBufferSize)
: EncBuffer(maxBufferSize + AES_BLOCK_SIZE, 0)
, DecBuffer(maxBufferSize + AES_BLOCK_SIZE, 0) {}

std::error_code TCrypt::Init(const std::string& cipher, const std::string& iv) noexcept {
    if (!DecodeCtx) {
        return EErrorCode::KeySize;
    }

    if (cipher.empty()) {
        return EErrorCode::KeySize;
    }

    if (iv.empty()) {
        return EErrorCode::IvSize;
    }

    TBuffer decodeCipher(cipher.size(), 0);
    std::int32_t decodeCipherSize;
    std::int32_t decodeCipherSizeFinal;

    EVP_DecodeInit(DecodeCtx.get());
    if(
        auto ret = EVP_DecodeUpdate(
            DecodeCtx.get(),
            decodeCipher.data(),
            &decodeCipherSize,
            reinterpret_cast<const unsigned char*>(cipher.c_str()),
            cipher.size()
        );
        ret == -1
    ) {
        return EErrorCode::InitKey;
    }

    if(
        auto ret = EVP_DecodeFinal(DecodeCtx.get(), decodeCipher.data() + decodeCipherSize, &decodeCipherSizeFinal);
        ret == -1
    ) {
        return EErrorCode::InitKey;
    }

    if (decodeCipherSize + decodeCipherSizeFinal != 16) {
        return EErrorCode::KeySize;
    }

    TBuffer decodeIv(iv.size(), 0);
    std::int32_t decodeIvSize;
    std::int32_t decodeIvSizeFinal;

    EVP_DecodeInit(DecodeCtx.get());
    if(
        auto ret = EVP_DecodeUpdate(
            DecodeCtx.get(),
            decodeIv.data(),
            &decodeIvSize,
            reinterpret_cast<const unsigned char*>(iv.c_str()),
            iv.size()
        );
        ret == -1
    ) {
        return EErrorCode::InitIv;
    }

    if(
        auto ret = EVP_DecodeFinal(DecodeCtx.get(), decodeIv.data() + decodeIvSize, &decodeIvSizeFinal);
        ret == -1
    ) {
        return EErrorCode::InitIv;
    }

    if (decodeIvSize + decodeIvSizeFinal != 16) {
        return EErrorCode::IvSize;
    }

    if (auto ret = EVP_EncryptInit_ex(EncCtx.get(), EVP_aes_128_cbc(), nullptr, decodeCipher.data(), decodeIv.data()); ret == 0) {
        return EErrorCode::InitEncrypt;
    }

    if (auto ret = EVP_DecryptInit_ex(DecCtx.get(), EVP_aes_128_cbc(), nullptr, decodeCipher.data(), decodeIv.data()); ret == 0) {
        return EErrorCode::InitDecrypt;
    }

    return {};
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
