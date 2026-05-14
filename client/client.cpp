#include <configs.h>
#include <context/context.h>
#include <crypt/crypt.h>
#include <crypt/utils.h>
#include <epoll/epoll.h>
#include <socket/socket.h>
#include <tun/tun.h>
#include <utils/utils.h>
#include <types.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

void readTun(
    NContext::TContextPtr ctx,
    NTun::TTunPtr tun,
    NSocket::TSocketPtr socket,
    NCrypt::TCryptPtr crypt,
    std::string remoteIp,
    std::uint16_t remotePort
) {
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

void readSocket(
    NContext::TContextPtr ctx,
    NTun::TTunPtr tun,
    NSocket::TSocketPtr socket,
    NCrypt::TCryptPtr crypt,
    std::string remoteIp,
    std::uint16_t remotePort
) {
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
    std::size_t mtu = 0;
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
        remotePort = std::stoi(env.at("remotePort"));
    } else {
        std::cerr << "export REMOTE_PORT env" << std::endl;
        return 3;
    }

    if (env.count("localPort") > 0) {
        localPort = std::stoi(env.at("localPort"));
    } else {
        std::cerr << "export LOCAL_PORT env" << std::endl;
        return 4;
    }

    if (env.count("mtu") > 0) {
        mtu = std::stoi(env.at("mtu"));
        if (mtu > MAX_MTU_SIZE) {
            std::cerr << "mtu must less then MAX_MTU_SIZE" << std::endl;
            return 5;
        }
    } else {
        std::cerr << "export MTU env" << std::endl;
        return 6;
    }

    if (env.count("keysFile") > 0) {
        keysFile = env.at("keysFile");
    } else {
        std::cerr << "export KEYS_FILE env" << std::endl;
        return 7;
    }

    auto tun = std::make_shared<NTun::TTun>(MAX_DATA_SIZE);

    if (auto ec = tun->Init(tunDevice); ec) {
        std::cerr << ec.message() << std::endl;
        return 8;
    }

    auto socket = std::make_shared<NSocket::TSocket>(MAX_DATA_SIZE);

    if (auto ec = socket->Init(localIp, localPort); ec) {
        std::cerr << ec.message() << std::endl;
        return 9;
    }

    auto epoll = std::make_shared<NEpoll::TEpoll>(MAX_EVENTS);

    if (auto ec = epoll->Init(); ec) {
        std::cerr << ec.message() << std::endl;
        return 10;
    }

    if(auto ec = epoll->Add(tun); ec) {
        std::cerr << ec.message() << std::endl;
        return 11;
    }

    if(auto ec = epoll->Add(socket); ec) {
        std::cerr << ec.message() << std::endl;
        return 12;
    }

    auto ctx = std::make_shared<NContext::TContext>();

    auto crypt = std::make_shared<NCrypt::TCrypt>(MAX_DATA_SIZE);

    const auto keyPair = NCrypt::loadKeyPair(keysFile);

    if (!keyPair) {
        std::cerr << "failed load key pair" << std::endl;
        return 13;
    }

    if (auto ec = crypt->Init(keyPair->first, keyPair->second); ec) {
        std::cerr << ec.message() << std::endl;
        return 14;
    }

    std::thread tTun(readTun, ctx, tun, socket, crypt, remoteIp, remotePort);
    std::thread tSocket(readSocket, ctx, tun, socket, crypt, remoteIp, remotePort);

    while (true) {
        const auto numberFd = epoll->Wait();
        if (numberFd <= 0) {
            continue;
        }
        const auto& events = epoll->GetEvents();

        for (std::size_t index = 0; index < numberFd; ++index) {
            if (tun->IsFd(events[index].data.fd)) {
                ctx->TunNotify();
            }
            if (socket->IsFd(events[index].data.fd)) {
                ctx->SocketNotify();
            }
        }
    }
}
