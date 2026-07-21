#include "crypt.h"

#include <errors.h>

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <algorithm>
#include <utility>

namespace NCrypt {

TCrypt::~TCrypt() {
    if (EncCtx) {
        EVP_CIPHER_CTX_reset(EncCtx.get());
    }

    if (DecCtx) {
        EVP_CIPHER_CTX_reset(DecCtx.get());
    }
}

TCrypt::TCrypt(std::size_t maxBufferSize)
: EncBuffer(maxBufferSize + GCM_IV_TAG_SIZE, 0)
, DecBuffer(maxBufferSize, 0) {}

std::error_code TCrypt::Init(const std::string& key) noexcept {
    if (!DecodeCtx) {
        return EErrorCode::KeySize;
    }

    if (key.empty()) {
        return EErrorCode::KeySize;
    }

    TBuffer decodeKey(key.size(), 0);
    std::int32_t decodeKeySize = 0;
    std::int32_t decodeKeySizeFinal = 0;

    EVP_DecodeInit(DecodeCtx.get());
    if (
        auto ret = EVP_DecodeUpdate(
            DecodeCtx.get(),
            decodeKey.data(),
            &decodeKeySize,
            reinterpret_cast<const unsigned char*>(key.c_str()),
            key.size()
        );
        ret <= 0
    ) {
        return EErrorCode::InitKey;
    }

    if (
        auto ret = EVP_DecodeFinal(DecodeCtx.get(), decodeKey.data() + decodeKeySize, &decodeKeySizeFinal);
        ret <= 0
    ) {
        return EErrorCode::InitKey;
    }

    if (!std::in_range<std::size_t>(decodeKeySize + decodeKeySizeFinal)) {
        return EErrorCode::KeySize;
    }

    const size_t keySize = decodeKeySize + decodeKeySizeFinal;

    if (keySize != AES_KEY_SIZE) {
        return EErrorCode::KeySize;
    }

    if (EVP_EncryptInit_ex(EncCtx.get(), EVP_aes_128_gcm(), nullptr, decodeKey.data(), nullptr) != 1) {
        return EErrorCode::InitEncrypt;
    }

    if (EVP_DecryptInit_ex(DecCtx.get(), EVP_aes_128_gcm(), nullptr, decodeKey.data(), nullptr) != 1) {
        return EErrorCode::InitDecrypt;
    }

    std::fill(decodeKey.begin(), decodeKey.end(), 0);

    return {};
}

TBufferView TCrypt::Encrypt(TBufferView buffer) noexcept {
    if (!EncCtx || buffer.size() == 0 || !std::in_range<std::int32_t>(buffer.size())) {
        return {};
    }

    if (buffer.size() > EncBuffer.size() - GCM_IV_TAG_SIZE) {
        return {};
    }

    auto* iv = EncBuffer.data();

    if (RAND_bytes(iv, GCM_IV_SIZE) != 1) {
        return {};
    }

    if (EVP_EncryptInit_ex(EncCtx.get(), nullptr, nullptr, nullptr, iv) != 1) {
        return {};
    }

    auto* ciphertext = EncBuffer.data() + GCM_IV_SIZE;

    std::int32_t sizeEnc = 0;
    if (EVP_EncryptUpdate(EncCtx.get(), ciphertext, &sizeEnc, buffer.data(), buffer.size()) != 1) {
        return {};
    }

    std::int32_t sizeEncFinal = 0;
    if (EVP_EncryptFinal_ex(EncCtx.get(), ciphertext + sizeEnc, &sizeEncFinal) != 1) {
        return {};
    }

    const std::int32_t ciphertextLen = sizeEnc + sizeEncFinal;

    if (EVP_CIPHER_CTX_ctrl(EncCtx.get(), EVP_CTRL_GCM_GET_TAG, GCM_TAG_SIZE, ciphertext + ciphertextLen) != 1) {
        return {};
    }

    if (!std::in_range<std::size_t>(ciphertextLen)) {
        return {};
    }

    return {EncBuffer.begin(), ciphertextLen + GCM_IV_TAG_SIZE};
}

TBufferView TCrypt::Decrypt(TBufferView buffer) noexcept {
    if (!DecCtx || buffer.size() == 0 || !std::in_range<std::int32_t>(buffer.size())) {
        return {};
    }

    if (buffer.size() <= GCM_IV_TAG_SIZE || buffer.size() > DecBuffer.size() + GCM_IV_TAG_SIZE) {
        return {};
    }

    auto* iv = buffer.data();

    if (EVP_DecryptInit_ex(DecCtx.get(), nullptr, nullptr, nullptr, iv) != 1) {
        return {};
    }

    auto* tag = const_cast<std::uint8_t*>(buffer.data()) + buffer.size() - GCM_TAG_SIZE;

    if (EVP_CIPHER_CTX_ctrl(DecCtx.get(), EVP_CTRL_GCM_SET_TAG, GCM_TAG_SIZE, tag) != 1) {
        return {};
    }

    auto* ciphertext = buffer.data() + GCM_IV_SIZE;
    const std::int32_t ciphertextLen = buffer.size() - GCM_IV_TAG_SIZE;

    std::int32_t sizeDec = 0;
    if (EVP_DecryptUpdate(DecCtx.get(), DecBuffer.data(), &sizeDec, ciphertext, ciphertextLen) != 1) {
        return {};
    }

    std::int32_t sizeDecFinal = 0;
    if (EVP_DecryptFinal_ex(DecCtx.get(), DecBuffer.data() + sizeDec, &sizeDecFinal) != 1) {
        return {};
    }

    if (!std::in_range<std::size_t>(sizeDec + sizeDecFinal)) {
        return {};
    }

    const std::size_t totalLen = sizeDec + sizeDecFinal;

    return {DecBuffer.begin(), totalLen};
}

}
