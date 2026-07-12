#include <configs.h>
#include <context/context.h>
#include <crypt/crypt.h>
#include <socket/socket.h>
#include <poll/poll.h>
#include <tun/tun.h>
#include <utils/utils.h>

#include <iostream>
#include <memory>
#include <thread>

void tx(
    NContext::TContextPtr ctx,
    NTun::TTunPtr tun,
    NSocket::TSocketPtr socket,
    NCrypt::TCryptPtr crypt,
    TConf conf
) noexcept {
    while(true) {
        ctx->TunWait();
        const auto& [buffer, size] = tun->Read();
        if (size == 0) {
            continue;
        } else if (!NUtils::ValidTunFrame(buffer, size)) {
            continue;
        }
        const auto& [encrBuffer, encrSize] = crypt->Encrypt(buffer, size);
        socket->Write(encrBuffer, encrSize, conf.RemoteIp, conf.RemotePort);
    }
}

void rx(
    NContext::TContextPtr ctx,
    NTun::TTunPtr tun,
    NSocket::TSocketPtr socket,
    NCrypt::TCryptPtr crypt,
    TConf conf
) noexcept {
    while(true) {
        ctx->SocketWait();
        const auto& [buffer, size, ip, port] = socket->Read();
        if (size == 0) {
            continue;
        } else if (conf.RemoteIp != ip || port != conf.RemotePort) {
            continue;
        }
        const auto& [decrBuffer, decrSize] = crypt->Decrypt(buffer, size);
        if (!NUtils::ValidTunFrame(decrBuffer, decrSize)) {
            continue;
        }
        tun->Write(decrBuffer, decrSize);
    }
}

int main() {
    try {
        const auto& [ec, conf] = NUtils::GetConf();

        if (ec) {
             std::cerr << "get conf error: " << ec.message() << std::endl;
             return 1;
        }

        auto ctx = std::make_shared<NContext::TContext>();

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

        auto poll = std::make_shared<NPoll::TPoll>(MAX_POLL_EVENTS, MAX_POLL_TIMEOUT_MS);

        if (auto ec = poll->Init(); ec) {
            std::cerr << ec.message() << std::endl;
            return 4;
        }

        if (auto ec = poll->RegisterHandlerIn(tun, [ctx] { ctx->TunNotify(); }); ec) {
            std::cerr << ec.message() << std::endl;
            return 5;
        }

        if (auto ec = poll->RegisterHandlerIn(socket, [ctx] { ctx->SocketNotify(); }); ec) {
            std::cerr << ec.message() << std::endl;
            return 6;
        }

        auto crypt = std::make_shared<NCrypt::TCrypt>(MAX_DATA_SIZE);

        const auto keyPair = NUtils::LoadKeyPair(conf.KeysFile);

        if (!keyPair) {
            std::cerr << "failed load key pair" << std::endl;
            return 7;
        }

        if (auto ec = crypt->Init(keyPair->first, keyPair->second); ec) {
            std::cerr << ec.message() << std::endl;
            return 8;
        }

        std::thread tTx(tx, ctx, tun, socket, crypt, conf);
        std::thread tRx(rx, ctx, tun, socket, crypt, conf);

        std::cerr << "run client" << std::endl;

        while (true) {
            poll->RunOne();
        }
    } catch(const std::exception& exc) {
        std::cerr << exc.what() << std::endl;
        return 9;
    }
}
