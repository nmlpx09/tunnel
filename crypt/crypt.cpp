#include "crypt.h"

#include <errors.h>

#include <openssl/aes.h>
#include <openssl/evp.h>

#include <utility>

namespace NCrypt {

TCrypt::TCrypt(std::size_t maxBytesSize)
: EncBytes(maxBytesSize + AES_BLOCK_SIZE, 0)
, DecBytes(maxBytesSize + AES_BLOCK_SIZE, 0) {}

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

    TBytes decodeCipher(cipher.size(), 0);
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

    TBytes decodeIv(iv.size(), 0);
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

TBuffer TCrypt::Encrypt(TBuffer buffer) noexcept {
    if (!EncCtx) {
        return {};
    }

    if (buffer.size() > EncBytes.size()) {
        return {};
    }

    EVP_EncryptInit_ex(EncCtx.get(), nullptr, nullptr, nullptr, nullptr);

    std::int32_t sizeEnc;
    if (auto ret = EVP_EncryptUpdate(EncCtx.get(), EncBytes.data(), &sizeEnc, buffer.data(), buffer.size()); ret == 0) {
        return {};
    }

    std::int32_t sizeEncFinal;
    if (auto ret = EVP_EncryptFinal_ex(EncCtx.get(), EncBytes.data() + sizeEnc, &sizeEncFinal); ret == 0) {
        return {};
    }

    sizeEnc += sizeEncFinal;

    if (!std::in_range<std::size_t>(sizeEnc)) {
        return {};
    }

    return {EncBytes.begin(), static_cast<std::size_t>(sizeEnc)};
}

TBuffer TCrypt::Decrypt(TBuffer buffer) noexcept {
    if (!DecCtx) {
        return {};
    }

    if (buffer.size() > DecBytes.size()) {
        return {};
    }

    EVP_DecryptInit_ex(DecCtx.get(), nullptr, nullptr, nullptr, nullptr);

    std::int32_t sizeDec;
    if (auto ret = EVP_DecryptUpdate(DecCtx.get(), DecBytes.data(), &sizeDec, buffer.data(), buffer.size()); ret == 0) {
        return {};
    }

    std::int32_t sizeDecFinal;
    if (auto ret = EVP_DecryptFinal_ex(DecCtx.get(), DecBytes.data() + sizeDec, &sizeDecFinal); ret == 0) {
        return {};
    }

    sizeDec += sizeDecFinal;

    if (!std::in_range<std::size_t>(sizeDec)) {
        return {};
    }

    return {DecBytes.begin(), static_cast<std::size_t>(sizeDec)};
}

}
