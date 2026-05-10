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
        const auto [size, host, port] = socket->Read();
        const auto& buffer = socket->GetBuffer();
        if (size == 0) {
            ctx->SocketReset();
            continue;
        } else if (!NUtils::validIpDatagram(buffer, size)) {
            continue;
        }

        ipsStorage->Add(NUtils::getSrcIp(buffer), host, port);

        tun->Write(buffer, size);
    }
}

int main() {
    std::string tunDevice = "tun0";
    std::string localHost = "0.0.0.0";
    std::uint16_t localPort = 1234;
    std::size_t dataSize = 1500;
    std::size_t maxEvents = 2;

    auto tun = std::make_shared<NTun::TTun>(dataSize);

    if(auto ret = tun->Init(tunDevice); ret < 0) {
        std::cerr << "failed tunnel init:" << strerror(errno) << std::endl;
        return 1;
    }

    auto socket = std::make_shared<NSocket::TSocket>(dataSize);

    if(auto ret = socket->Init(localHost, localPort); ret < 0) {
        std::cerr << "failed socket init:" << strerror(errno) << std::endl;
        return 1;
    }

    auto epoll = std::make_shared<NEpoll::TEpoll>(maxEvents);

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