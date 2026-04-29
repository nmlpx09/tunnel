#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>


#include <iostream>
#include <string>
#include <vector>


int main() {
    std::string tun = "tun0";
    std::string localHost = "0.0.0.0";
    std::uint16_t localPort = 1234;
    std::size_t dataSize = 1400;
    std::vector<std::uint8_t> buf(dataSize, 0);
    sockaddr_in sockaddrServer, sockaddrClient;
    ifreq ifr;
    epoll_event events[2];

    std::int32_t tunfd = -1, sockfd = -1, epollfd = -1;

    if(tunfd = open("/dev/net/tun", O_RDWR); tunfd < 0) {
        std::cerr << "failed open tun\n";
        return 1;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN;
    strncpy(ifr.ifr_name, tun.c_str(), IFNAMSIZ);

    if (auto err = ioctl(tunfd, TUNSETIFF, (void*)&ifr); err == -1) {
        std::cerr << "failed ioctl TUNSETIFF\n"; 
        close(tunfd);
        return 1;
    }

    if (sockfd = socket(AF_INET, SOCK_DGRAM, 0); sockfd < 0) {
        std::cerr << "failed open tun\n";
        close(tunfd);
        return 1;
    }

    sockaddrServer = sockaddr_in {
        .sin_family = AF_INET,
        .sin_port = htons(localPort),
        .sin_addr = {
            .s_addr = inet_addr(localHost.c_str())
        },
        .sin_zero = {0}
    };

    if (auto br = bind(sockfd, reinterpret_cast<const sockaddr*>(&sockaddrServer), sizeof(sockaddrServer)); br < 0) {
        std::cerr << "failed bind socket\n";
        close(tunfd);
        close(sockfd);
        return 1;
    }

    fcntl(sockfd, F_SETFL, O_NONBLOCK);
    fcntl(tunfd, F_SETFL, O_NONBLOCK);

    if (epollfd = epoll_create1(0); epollfd < 0) {
        std::cerr << "failed epoll create\n";
        close(tunfd);
        close(sockfd);
        return 1;
    }

    epoll_event event;

    event.events = EPOLLIN;
    event.data.fd = sockfd;
    if (auto err = epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event); err < 0) {
        std::cerr << "failed epoll create\n";
        close(tunfd);
        close(sockfd);
        close(epollfd);
        return 1;
    }

    event.events = EPOLLIN;
    event.data.fd = tunfd;
    if (auto err = epoll_ctl(epollfd, EPOLL_CTL_ADD, tunfd, &event); err < 0) {
        std::cerr << "failed epoll create\n";
        close(tunfd);
        close(sockfd);
        close(epollfd);
        return 1;
    }

    for(;;) {
        auto nfds = epoll_wait(epollfd, events, 2, -1);
        if (nfds == -1) {
            std::cerr << "failed epoll wait\n";
            continue;
        }

        for (auto index = 0; index < nfds; ++index) {
            if (events[index].data.fd == tunfd) {
                auto readSize = read(tunfd, buf.data(), dataSize);
                if (readSize <= 0) {
                    continue;
                }
                sendto(sockfd, buf.data(), readSize, 0, reinterpret_cast<const sockaddr*>(&sockaddrClient), sizeof(sockaddrClient));
            }
            if (events[index].data.fd == sockfd) {
                std::uint32_t scl;
                auto readSize = recvfrom(sockfd, buf.data(), dataSize, 0, reinterpret_cast<sockaddr*>(&sockaddrClient), &scl);
                if (readSize <= 0) {
                    continue;
                }
                write(tunfd, buf.data(), readSize);
            }
        }
    }
}
