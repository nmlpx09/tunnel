#include <configs.h>
#include <context/context.h>
#include <common/types.h>
#include <common/utils.h>
#include <epoll/epoll.h>
#include <ips_storage/ips_storage.h>
#include <socket/socket.h>
#include <tun/tun.h>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

void readTun(
    NContext::TContextPtr ctx,
    NTun::TTunPtr tun,
    NSocket::TSocketPtr socket,
    NIpsStorage::TIpsStoragePtr ipsStorage
) {
    while(true) {
        ctx->TunWait();
        const auto size = tun->Read();
        const auto& buffer = tun->GetBuffer();
        if (size == 0) {
            ctx->TunReset();
            continue;
        } else if (!NUtils::validIpDatagram(buffer, size)) {
            continue;
        }

        if (const auto value = ipsStorage->Get(NUtils::getDstIp(buffer)); value) {
            socket->Write(buffer, size, value->first, value->second);
        }
    }
}

void readSocket(
    NContext::TContextPtr ctx,
    NTun::TTunPtr tun,
    NSocket::TSocketPtr socket,
    NIpsStorage::TIpsStoragePtr ipsStorage
) {
    while(true) {
        ctx->SocketWait();
        const auto [size, ip, port] = socket->Read();
        const auto& buffer = socket->GetBuffer();
        if (size == 0) {
            ctx->SocketReset();
            continue;
        } else if (!NUtils::validIpDatagram(buffer, size)) {
            continue;
        }

        ipsStorage->Add(NUtils::getSrcIp(buffer), ip, port);

        tun->Write(buffer, size);
    }
}

int main() {
    std::string tunDevice;
    const std::string localIp = "0.0.0.0";
    std::uint16_t localPort = 0;
    std::size_t mtu = 0;

    const auto env = NUtils::getEnv();

    if(env.count("tunDevice") > 0) {
        tunDevice = env.at("tunDevice");
    } else {
        std::cerr << "set TUN_DEVICE env" << std::endl;
        return 1;
    }

    if(env.count("localPort") > 0) {
        localPort = std::stoi(env.at("localPort"));
    } else {
        std::cerr << "set LOCAL_PORT env" << std::endl;
        return 1;
    }

    if(env.count("mtu") > 0) {
        mtu = std::stoi(env.at("mtu"));
        if (mtu > MAX_MTU_SIZE) {
            std::cerr << "mtu must less then MAX_MTU_SIZE" << std::endl;
            return 1;
        }
    } else {
        std::cerr << "set MTU env" << std::endl;
        return 1;
    }

    auto tun = std::make_shared<NTun::TTun>(MAX_DATA_SIZE);

    if(auto ret = tun->Init(tunDevice); ret < 0) {
        std::cerr << "failed tunnel init:" << strerror(errno) << std::endl;
        return 1;
    }

    auto socket = std::make_shared<NSocket::TSocket>(MAX_DATA_SIZE);

    if(auto ret = socket->Init(localIp, localPort); ret < 0) {
        std::cerr << "failed socket init:" << strerror(errno) << std::endl;
        return 1;
    }

    auto epoll = std::make_shared<NEpoll::TEpoll>(MAX_EVENTS);

    if(auto ret = epoll->Init(); ret < 0) {
        std::cerr << "failed epoll init:" << strerror(errno) << std::endl;
        return 1;
    }

    epoll->Add(tun);
    epoll->Add(socket);

    auto ctx = std::make_shared<NContext::TContext>();

    auto ipsStorage = std::make_shared<NIpsStorage::TIpsStorage>();

    std::thread tTun(readTun, ctx, tun, socket, ipsStorage);
    std::thread tSocket(readSocket, ctx, tun, socket, ipsStorage);

    while(true) {
        auto numberFd = epoll->Wait();
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