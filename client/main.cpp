#include <context/context.h>
#include <common/types.h>
#include <common/utils.h>
#include <epoll/epoll.h>
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
    std::string remoteHost,
    std::uint16_t remotePort
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
        socket->Write(buffer, size, remoteHost, remotePort);
    }
}

void readSocket(
    NContext::TContextPtr ctx,
    NTun::TTunPtr tun,
    NSocket::TSocketPtr socket,
    std::string remoteHost,
    std::uint16_t remotePort
) {
    while(true) {
        ctx->SocketWait();
        const auto [size, host, port] = socket->Read();
        const auto& buffer = socket->GetBuffer();
        if (size == 0) {
            ctx->SocketReset();
            continue;
        } else if (remoteHost != host || port != remotePort || !NUtils::validIpDatagram(buffer, size)) {
            continue;
        }
        tun->Write(buffer, size);
    }
}

int main() {
    std::string tunDevice = "tun0";
    std::string remoteHost = "77.91.92.110";
    std::string localHost = "0.0.0.0";
    std::uint16_t remotePort = 1234;
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

    std::thread tTun(readTun, ctx, tun, socket, remoteHost, remotePort);
    std::thread tSocket(readSocket, ctx, tun, socket, remoteHost, remotePort);

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
