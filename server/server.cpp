#include <configs.h>
#include <crypt/crypt.h>
#include <ips_storage/ips_storage.h>
#include <socket/socket.h>
#include <poll/poll.h>
#include <tun/tun.h>
#include <utils/utils.h>

#include <iostream>
#include <memory>
#include <thread>

void tx(
    NPoll::TPollPtr poll,
    NTun::TTunPtr tun,
    NSocket::TSocketPtr socket,
    NCrypt::TCryptPtr crypt,
    NIpsStorage::TIpsStoragePtr ipsStorage
) noexcept {
    while(true) {
        if (!poll->RunOne()) {
            continue;
        }
        const auto& [buffer, size] = tun->Read();
        if (size == 0) {
            continue;
        } else if (!NUtils::ValidTunFrame(buffer, size)) {
            continue;
        }
        if (const auto value = ipsStorage->Get(NUtils::GetDstIpFromTunFrame(buffer, size)); value) {
            const auto& [encrBuffer, encrSize] = crypt->Encrypt(buffer, size);
            socket->Write(encrBuffer, encrSize, value->first, value->second);
        }
    }
}

void rx(
    NPoll::TPollPtr poll,
    NTun::TTunPtr tun,
    NSocket::TSocketPtr socket,
    NCrypt::TCryptPtr crypt,
    NIpsStorage::TIpsStoragePtr ipsStorage
) noexcept {
    while(true) {
        if (!poll->RunOne()) {
            continue;
        }
        const auto& [buffer, size, ip, port] = socket->Read();
        if (size == 0) {
            continue;
        }
        const auto& [decrBuffer, decrSize] = crypt->Decrypt(buffer, size);
        if (!NUtils::ValidTunFrame(decrBuffer, decrSize)) {
            continue;
        }

        ipsStorage->Add(NUtils::GetSrcIpFromTunFrame(decrBuffer, decrSize), ip, port);

        tun->Write(decrBuffer, decrSize);
    }
}

int main() {
    try {
        const auto& [ec, conf] = NUtils::GetConf(false);

        if (ec) {
             std::cerr << "get conf error: " << ec.message() << std::endl;
             return 1;
        }

        auto tun = std::make_shared<NTun::TTun>(MAX_DATA_SIZE);

        if (auto ec = tun->Init(conf.TunDevice); ec) {
            std::cerr << ec.message() << std::endl;
            return 2;
        }

        auto socket = std::make_shared<NSocket::TSocket>(MAX_DATA_SIZE);

        if (auto ec = socket->Init(conf.LocalIp, conf.LocalPort); ec) {
            std::cerr << ec.message() << std::endl;
            return 3;
        }

        auto pollTun = std::make_shared<NPoll::TPoll>(MAX_POLL_TIMEOUT_MS);

        if (auto ec = pollTun->Init(); ec) {
            std::cerr << ec.message() << std::endl;
            return 4;
        }

        if (auto ec = pollTun->RegisterFd(tun); ec) {
            std::cerr << ec.message() << std::endl;
            return 5;
        }

        auto pollSocket = std::make_shared<NPoll::TPoll>(MAX_POLL_TIMEOUT_MS);

        if (auto ec = pollSocket->Init(); ec) {
            std::cerr << ec.message() << std::endl;
            return 6;
        }

        if (auto ec = pollSocket->RegisterFd(socket); ec) {
            std::cerr << ec.message() << std::endl;
            return 7;
        }

        auto ipsStorage = std::make_shared<NIpsStorage::TIpsStorage>();

        auto crypt = std::make_shared<NCrypt::TCrypt>(MAX_DATA_SIZE);

        auto keyPair = NUtils::LoadKeyPair(conf.KeysFile);

        if (!keyPair) {
            std::cerr << "failed load key pair" << std::endl;
            return 8;
        }

        if (auto ec = crypt->Init(keyPair->first, keyPair->second); ec) {
            std::cerr << ec.message() << std::endl;
            return 9;
        }

        std::thread tTx(tx, pollTun, tun, socket, crypt, ipsStorage);
        std::thread tRx(rx, pollSocket, tun, socket, crypt, ipsStorage);

        std::cerr << "run server" << std::endl;

        tTx.join();
        tRx.join();
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << std::endl;
        return 10;
    }
}
