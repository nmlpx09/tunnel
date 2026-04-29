#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <unistd.h>

#include <atomic>
#include <condition_variable>
#include <cstring>
#include <memory>
#include <mutex>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using TBuffer = std::vector<std::uint8_t>;

struct TContext {
public:
    void TunnelWait() {
        std::unique_lock<std::recursive_mutex> ulock{TunnelMutex};
        TunnelCv.wait(ulock, [this] { return TunnelReady.load(); });
        TunnelReady = false;
    }
    void SocketWait() {
        std::unique_lock<std::recursive_mutex> ulock{SocketMutex};
        SocketCv.wait(ulock, [this] { return SocketReady.load(); });
        SocketReady = false;
    }
    void TunnelNotify() {
        TunnelReady = true;
        TunnelCv.notify_one();
    }
    void SocketNotify() {
        SocketReady = true;
        SocketCv.notify_one();
    }
private:
    std::recursive_mutex TunnelMutex;
    std::recursive_mutex SocketMutex;
    std::condition_variable_any TunnelCv;
    std::condition_variable_any SocketCv;
    std::atomic<bool> TunnelReady = false;
    std::atomic<bool> SocketReady = false;
};

using TContextPtr = std::shared_ptr<TContext>;

struct TTunnel {
public:
    ~TTunnel() {
        close(Fd);
    }

    TTunnel(std::size_t maxBufferSize) : MaxBufferSize(maxBufferSize) { }

    std::int32_t Init(std::string deviceName) {
        if(Fd = open("/dev/net/tun", O_RDWR); Fd < 0) {
            return -1;
        }

        ifreq ifr;
        std::memset(&ifr, 0, sizeof(ifr));
        ifr.ifr_flags = IFF_TUN;
        std::strncpy(ifr.ifr_name, deviceName.c_str(), IFNAMSIZ);

        if (auto ret = ioctl(Fd, TUNSETIFF, &ifr); ret < 0) {
            return -1;
        }

        if (auto ret = fcntl(Fd, F_SETFL, O_NONBLOCK); ret < 0){
            return -1;
        }
        return 0;
    }

    std::int32_t Write(TBuffer buffer) const {
        if (Fd < 0) {
            return 0;
        }

        if (buffer.empty()) {
            return 0;
        }
        return write(Fd, buffer.data(), buffer.size());
    }

    TBuffer Read() const {
        if (Fd < 0) {
            return {};
        }

        TBuffer buffer(MaxBufferSize, 0);
        auto readSize = read(Fd, buffer.data(), buffer.size());

        if (readSize <= 0) {
            return {};
        }

        buffer.resize(readSize);
        return buffer;
    }

    bool IsFd(std::int32_t fd) {
        return Fd == fd;
    }
private:
    std::int32_t Fd = -1;
    std::size_t MaxBufferSize = 0;
    friend class TEpoll;
};

using TTunnelPtr = std::shared_ptr<TTunnel>;

struct TSocket {
public:
    ~TSocket() {
        close(Fd);
    }

    TSocket(std::size_t maxBufferSize) : MaxBufferSize(maxBufferSize) { }

    std::int32_t Init(
        std::string localHost,
        std::uint16_t localPort,
        std::string remoteHost,
        std::uint16_t remotePort
    ) {
        if (Fd = socket(AF_INET, SOCK_DGRAM, 0); Fd < 0) {
            return -1;
        }

        sockaddr_in sockaddrClient = sockaddr_in {
            .sin_family = AF_INET,
            .sin_port = htons(localPort),
            .sin_addr = {
                .s_addr = inet_addr(localHost.c_str())
            },
            .sin_zero = {0}
        };

        if (auto ret = bind(Fd,
            reinterpret_cast<const sockaddr*>(&sockaddrClient),
            sizeof(sockaddrClient)); ret < 0
        ) {
            return -1;
        }

        if (auto ret = fcntl(Fd, F_SETFL, O_NONBLOCK); ret < 0){
            return -1;
        }

        SockaddrServer = sockaddr_in {
            .sin_family = AF_INET,
            .sin_port = htons(remotePort),
            .sin_addr = {
                .s_addr = inet_addr(remoteHost.c_str())
            },
            .sin_zero = {0}
        };

        return 0;
    }

    std::int32_t Write(TBuffer buffer) const {
        if (Fd < 0) {
            return 0;
        }

        if (buffer.empty()) {
            return 0;
        }
        return sendto(Fd, buffer.data(), buffer.size(), 0,
            reinterpret_cast<const sockaddr*>(&SockaddrServer), sizeof(SockaddrServer));
    }

    TBuffer Read() const {
        if (Fd < 0) {
            return {};
        }

        TBuffer buffer(MaxBufferSize, 0);
        auto readSize = recv(Fd, buffer.data(), MaxBufferSize, 0);

        if (readSize <= 0) {
            return {};
        }

        buffer.resize(readSize);
        return buffer;
    }

    bool IsFd(std::int32_t fd) {
        return Fd == fd;
    }
private:
    std::int32_t Fd = -1;
    std::size_t MaxBufferSize = 0;
    sockaddr_in SockaddrServer;
    friend class TEpoll;
};

using TSocketPtr = std::shared_ptr<TSocket>;

struct TEpoll {
private:
    using TEvents = std::vector<epoll_event>;
public:
    ~TEpoll() {
        close(Fd);
    }

    TEpoll(std::size_t maxEvents) : MaxEvents(maxEvents) { }

    std::int32_t Init() {
        if (MaxEvents == 0) {
            return -1;
        }

        if (Fd = epoll_create1(0); Fd < 0) {
            return -1;
        }

        Events = std::vector<epoll_event>(MaxEvents);
        return 0;
    }

    template <class TFdProviderPtr>
    std::int32_t Add(TFdProviderPtr fdProvider) const {
        auto event = epoll_event {
            .events = EPOLLIN,
            .data = {
                .fd = fdProvider->Fd
            }
        };
        if (auto ret = epoll_ctl(Fd, EPOLL_CTL_ADD, fdProvider->Fd, &event); ret < 0) {
            return -1;
        }
        return 0;
    }

    std::size_t Wait() {
        return epoll_wait(Fd, &Events[0], MaxEvents, -1);
    }

    const TEvents& GetEvents() const {
        return Events;
    }

private:
    std::int32_t Fd = -1;
    std::size_t MaxEvents = 0;
    TEvents Events;
};

using TEpollPtr = std::shared_ptr<TEpoll>;

void readTunnel(TContextPtr ctx, TTunnelPtr tunnel, TSocketPtr socket) {
    while(true) {
        ctx->TunnelWait();
        auto buffer = tunnel->Read();
        socket->Write(std::move(buffer));
    }
}

void readSocket(TContextPtr ctx, TTunnelPtr tunnel, TSocketPtr socket) {
    while(true) {
        ctx->SocketWait();
        auto buffer = socket->Read();
        tunnel->Write(std::move(buffer));
    }
}

int main() {
    std::string tun = "tun0";
    std::string remoteHost = "77.91.92.110";
    std::string localHost = "0.0.0.0";
    std::uint16_t remotePort = 1234;
    std::uint16_t localPort = 1234;
    std::size_t dataSize = 1400;
    std::size_t maxEvents = 10;

    auto tunnel = std::make_shared<TTunnel>(dataSize);

    if(auto ret = tunnel->Init(tun); ret < 0) {
        std::cerr << "failed tunnel init:" << strerror(errno) << std::endl;
        return 1;
    }

    auto socket = std::make_shared<TSocket>(dataSize);

    if(auto ret = socket->Init(localHost, localPort, remoteHost, remotePort); ret < 0) {
        std::cerr << "failed socket init:" << strerror(errno) << std::endl;
        return 1;
    }

    auto epoll = std::make_shared<TEpoll>(maxEvents);

    if(auto ret = epoll->Init(); ret < 0) {
        std::cerr << "failed epoll init:" << strerror(errno) << std::endl;
        return 1;
    }

    epoll->Add(tunnel);
    epoll->Add(socket);

    auto ctx = std::make_shared<TContext>();

    std::thread tTunnel(readTunnel, ctx, tunnel, socket);
    std::thread tSocket(readSocket, ctx, tunnel, socket);

    while(true) {
        auto numberFd = epoll->Wait();
        if (numberFd <=0) {
            continue;
        }
        const auto& events = epoll->GetEvents();

        for (auto index = 0; index < numberFd; ++index) {
            if (tunnel->IsFd(events[index].data.fd)) {
                ctx->TunnelNotify();
            }
            if (socket->IsFd(events[index].data.fd)) {
                ctx->SocketNotify();
            }
        }
    }
}
