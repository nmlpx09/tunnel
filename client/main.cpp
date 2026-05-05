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
    void TunWait() {
        std::unique_lock<std::mutex> ulock{TunMutex};
        TunCv.wait(ulock, [this] { return TunReady > 0; });
        --TunReady;
    }
    void SocketWait() {
        std::unique_lock<std::mutex> ulock{SocketMutex};
        SocketCv.wait(ulock, [this] { return SocketReady > 0; });
        --SocketReady;
    }
    void TunNotify() {
        std::unique_lock<std::mutex> ulock{TunMutex};
        ++TunReady;
        TunCv.notify_one();
    }
    void SocketNotify() {
        std::unique_lock<std::mutex> ulock{SocketMutex};
        ++SocketReady;
        SocketCv.notify_one();
    }
private:
    std::mutex TunMutex;
    std::mutex SocketMutex;
    std::condition_variable TunCv;
    std::condition_variable SocketCv;
    std::size_t TunReady = 0;
    std::size_t SocketReady = 0;
};

using TContextPtr = std::shared_ptr<TContext>;

struct TTun {
public:
    ~TTun() {
        close(Fd);
    }

    TTun(std::size_t maxBufferSize)
    : MaxBufferSize(maxBufferSize)
    , Buffer(MaxBufferSize, 0) { }

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

    void Write(const TBuffer& buffer, std::size_t size) const {
        if (Fd < 0) {
            return;
        }

        if (size == 0) {
            return;
        }
        write(Fd, buffer.data(), size);
    }

    std::size_t Read() {
        if (Fd < 0) {
            return 0;
        }

        auto readSize = read(Fd, Buffer.data(), MaxBufferSize);

        if (readSize <= 0) {
            return 0;
        }

        return readSize;
    }

    const TBuffer& getBuffer() const {
        return Buffer;
    }

    bool IsFd(std::int32_t fd) const {
        return Fd == fd;
    }

private:
    std::int32_t Fd = -1;
    std::size_t MaxBufferSize = 0;
    TBuffer Buffer;
    friend class TEpoll;
};

using TTunPtr = std::shared_ptr<TTun>;

struct TSocket {
public:
    ~TSocket() {
        close(Fd);
    }

    TSocket(std::size_t maxBufferSize)
    : MaxBufferSize(maxBufferSize)
    , Buffer(MaxBufferSize, 0) { }

    std::int32_t Init(
        std::string localHost,
        std::uint16_t localPort
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

        return 0;
    }

    void Write(
        const TBuffer& buffer,
        std::size_t size,
        const std::string& remoteHost,
        std::uint16_t remotePort
    ) const {
        if (Fd < 0) {
            return;
        }

        if (size == 0) {
            return;
        }

        auto sockaddrRemote = sockaddr_in {
            .sin_family = AF_INET,
            .sin_port = htons(remotePort),
            .sin_addr = {
                .s_addr = inet_addr(remoteHost.c_str())
            },
            .sin_zero = {0}
        };

        sendto(Fd, buffer.data(), size, 0,
            reinterpret_cast<const sockaddr*>(&sockaddrRemote), sizeof(sockaddrRemote));
    }

    std::tuple<std::size_t, std::string, std::uint16_t> Read() {
        if (Fd < 0) {
            return {0, {}, 0};
        }

        sockaddr_in sockaddrRemote;
        std::uint32_t sockaddrSize;

        auto readSize = recvfrom(Fd, Buffer.data(), MaxBufferSize, 0, reinterpret_cast<sockaddr*>(&sockaddrRemote), &sockaddrSize);

        if (readSize <= 0) {
            return {0, {}, 0};
        }

        return {readSize, inet_ntoa(sockaddrRemote.sin_addr), htons(sockaddrRemote.sin_port)};
    }

    const TBuffer& getBuffer() const {
        return Buffer;
    }

    bool IsFd(std::int32_t fd) const {
        return Fd == fd;
    }

private:
    std::int32_t Fd = -1;
    std::size_t MaxBufferSize = 0;
    TBuffer Buffer;
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

bool isValidIpData(const TBuffer& buffer) {
    return *reinterpret_cast<const std::uint32_t*>(buffer.data()) == 0x80000;
}

void readTun(
    TContextPtr ctx,
    TTunPtr tun,
    TSocketPtr socket,
    std::string remoteHost,
    std::uint16_t remotePort
) {
    while(true) {
        ctx->TunWait();
        auto size = tun->Read();
        const auto& buffer = tun->getBuffer();
        if (size < 4 || !isValidIpData(buffer)) {
            continue;
        }
        socket->Write(buffer, size, remoteHost, remotePort);
    }
}

void readSocket(
    TContextPtr ctx,
    TTunPtr tun,
    TSocketPtr socket,
    std::string remoteHost,
    std::uint16_t remotePort
) {
    while(true) {
        ctx->SocketWait();
        auto [size, host, port] = socket->Read();
        const auto& buffer = socket->getBuffer();
        if (size < 4 || remoteHost != host || port != remotePort || !isValidIpData(buffer)) {
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

    auto tun = std::make_shared<TTun>(dataSize);

    if(auto ret = tun->Init(tunDevice); ret < 0) {
        std::cerr << "failed tunnel init:" << strerror(errno) << std::endl;
        return 1;
    }

    auto socket = std::make_shared<TSocket>(dataSize);

    if(auto ret = socket->Init(localHost, localPort); ret < 0) {
        std::cerr << "failed socket init:" << strerror(errno) << std::endl;
        return 1;
    }

    auto epoll = std::make_shared<TEpoll>(maxEvents);

    if(auto ret = epoll->Init(); ret < 0) {
        std::cerr << "failed epoll init:" << strerror(errno) << std::endl;
        return 1;
    }

    epoll->Add(tun);
    epoll->Add(socket);

    auto ctx = std::make_shared<TContext>();

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
