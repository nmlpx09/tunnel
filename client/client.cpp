#include <configs.h>
#include <context/context.h>
#include <crypt/crypt.h>
#include <socket/socket.h>
#include <poll/poll.h>
#include <tun/tun.h>
#include <utils/utils.h>
#include <types.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

void tx(
    NContext::TContextPtr ctx,
    NTun::TTunPtr tun,
    NSocket::TSocketPtr socket,
    NCrypt::TCryptPtr crypt,
    std::string remoteIp,
    std::uint16_t remotePort
) noexcept {
    while(true) {
        ctx->TunWait();
        const auto& [buffer, size] = tun->Read();
        if (size == 0) {
            ctx->TunReset();
            continue;
        } else if (!NUtils::validIpDatagram(buffer, size)) {
            continue;
        }
        const auto& [encrBuffer, encrSize] = crypt->Encrypt(buffer, size);
        socket->Write(encrBuffer, encrSize, remoteIp, remotePort);
    }
}

void rx(
    NContext::TContextPtr ctx,
    NTun::TTunPtr tun,
    NSocket::TSocketPtr socket,
    NCrypt::TCryptPtr crypt,
    std::string remoteIp,
    std::uint16_t remotePort
) noexcept {
    while(true) {
        ctx->SocketWait();
        const auto& [buffer, size, ip, port] = socket->Read();
        if (size == 0) {
            ctx->SocketReset();
            continue;
        } else if (remoteIp != ip || port != remotePort) {
            continue;
        }
        const auto& [decrBuffer, decrSize] = crypt->Decrypt(buffer, size);
        if (!NUtils::validIpDatagram(decrBuffer, decrSize)) {
            continue;
        }
        tun->Write(decrBuffer, decrSize);
    }
}

int main() {
    std::string tunDevice;
    std::string remoteIp;
    std::uint16_t remotePort = 0;
    const std::string localIp = "0.0.0.0";
    std::uint16_t localPort = 0;
    std::size_t tunMtu = 0;
    std::string keysFile;

    const auto env = NUtils::getEnv();

    if (env.count("tunDevice") > 0) {
        tunDevice = env.at("tunDevice");
    } else {
        std::cerr << "export TUN_DEVICE env" << std::endl;
        return 1;
    }

    if (env.count("remoteIp") > 0) {
        remoteIp = env.at("remoteIp");
    } else {
        std::cerr << "export REMOTE_IP env" << std::endl;
        return 2;
    }

    if (env.count("remotePort") > 0) {
        try {
            remotePort = std::stoi(env.at("remotePort"));
        } catch (const std::exception&) {
            std::cerr << "error convert REMOTE_PORT from string" << std::endl;
            return 3;
        }
    } else {
        std::cerr << "export REMOTE_PORT env" << std::endl;
        return 4;
    }

    if (env.count("localPort") > 0) {
        try {
            localPort = std::stoi(env.at("localPort"));
        } catch (const std::exception&) {
            std::cerr << "error convert LOCAL_PORT from string" << std::endl;
            return 5;
        }
    }

    if (env.count("tunMtu") > 0) {
        try {
            tunMtu = std::stoi(env.at("tunMtu"));
        } catch (const std::exception&) {
            std::cerr << "error convert TUN_MTU from string" << std::endl;
            return 6;
        }
        if (tunMtu > MAX_TUN_MTU_SIZE) {
            std::cerr << "tun mtu must less then TUN_MTU" << std::endl;
            return 7;
        }
    } else {
        std::cerr << "export TUN_MTU env" << std::endl;
        return 8;
    }

    if (env.count("keysFile") > 0) {
        keysFile = env.at("keysFile");
    } else {
        std::cerr << "export KEYS_FILE env" << std::endl;
        return 9;
    }

    auto ctx = std::make_shared<NContext::TContext>();

    auto tun = std::make_shared<NTun::TTun>(MAX_DATA_SIZE);

    if (auto ec = tun->Init(tunDevice); ec) {
        std::cerr << ec.message() << std::endl;
        return 10;
    }

    auto socket = std::make_shared<NSocket::TSocket>(MAX_DATA_SIZE);

    if (auto ec = socket->Init(localIp, localPort); ec) {
        std::cerr << ec.message() << std::endl;
        return 11;
    }

    auto poll = std::make_shared<NPoll::TPoll>(MAX_POLL_EVENTS, MAX_POLL_TIMEOUT_MS);

    if (auto ec = poll->Init(); ec) {
        std::cerr << ec.message() << std::endl;
        return 12;
    }

    if (auto ec = poll->RegisterHandlerIn(tun, [ctx] { ctx->TunNotify(); }); ec) {
        std::cerr << ec.message() << std::endl;
        return 13;
    }

    if (auto ec = poll->RegisterHandlerIn(socket, [ctx] { ctx->SocketNotify(); }); ec) {
        std::cerr << ec.message() << std::endl;
        return 14;
    }

    auto crypt = std::make_shared<NCrypt::TCrypt>(MAX_DATA_SIZE);

    const auto keyPair = NUtils::loadKeyPair(keysFile);

    if (!keyPair) {
        std::cerr << "failed load key pair" << std::endl;
        return 15;
    }

    if (auto ec = crypt->Init(keyPair->first, keyPair->second); ec) {
        std::cerr << ec.message() << std::endl;
        return 16;
    }

    std::thread tTx(tx, ctx, tun, socket, crypt, remoteIp, remotePort);
    std::thread tRx(rx, ctx, tun, socket, crypt, remoteIp, remotePort);

    std::cerr << "run client" << std::endl;

    while (true) {
        poll->RunOne();
    }
}
