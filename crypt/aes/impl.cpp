#include "impl.h"

#include <configs.h>
#include <errors.h>

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <algorithm>
#include <utility>
#include <vector>

namespace NCrypt {

TAes::~TAes() {
    if (EncCtx) {
        EVP_CIPHER_CTX_reset(EncCtx.get());
    }

    if (DecCtx) {
        EVP_CIPHER_CTX_reset(DecCtx.get());
    }
}

std::error_code TAes::Init(const std::string& key) {
    if (!DecodeCtx) {
        return EErrorCode::KeySize;
    }

    if (key.empty()) {
        return EErrorCode::KeySize;
    }

    std::vector<std::uint8_t> decodeKey(key.size(), 0);
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
        ret == -1
    ) {
        return EErrorCode::InitKey;
    }

    if (
        auto ret = EVP_DecodeFinal(DecodeCtx.get(), decodeKey.data() + decodeKeySize, &decodeKeySizeFinal);
        ret == -1
    ) {
        return EErrorCode::InitKey;
    }

    if (!std::in_range<std::size_t>(decodeKeySize + decodeKeySizeFinal)) {
        return EErrorCode::KeySize;
    }

    const size_t keySize = decodeKeySize + decodeKeySizeFinal;

    if (keySize != GCM_KEY_SIZE) {
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

TBufferView TAes::Encrypt(TBufferView buffer) noexcept {
    if (!EncCtx || buffer.empty() || !std::in_range<std::int32_t>(buffer.size())) {
        return {};
    }

    if (buffer.size() + GCM_OVERHEAD > EncBuffer.size()) {
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

    const std::size_t totalLen = ciphertextLen + GCM_OVERHEAD;

    return {EncBuffer.begin(), totalLen};
}

TBufferView TAes::Decrypt(TBufferView buffer) noexcept {
    if (!DecCtx || buffer.empty() || !std::in_range<std::int32_t>(buffer.size())) {
        return {};
    }

    if (buffer.size() <= GCM_OVERHEAD || buffer.size() > DecBuffer.size() + GCM_OVERHEAD) {
        return {};
    }

    const auto* iv = buffer.data();

    if (EVP_DecryptInit_ex(DecCtx.get(), nullptr, nullptr, nullptr, iv) != 1) {
        return {};
    }

    auto* tag = buffer.data() + buffer.size() - GCM_TAG_SIZE;

    if (EVP_CIPHER_CTX_ctrl(DecCtx.get(), EVP_CTRL_GCM_SET_TAG, GCM_TAG_SIZE, tag) != 1) {
        return {};
    }

    const auto* ciphertext = buffer.data() + GCM_IV_SIZE;
    const std::int32_t ciphertextLen = buffer.size() - GCM_OVERHEAD;

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
