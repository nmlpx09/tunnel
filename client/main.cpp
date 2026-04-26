#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/select.h>
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
    std::string remoteHost = "77.91.92.110";
    std::string localHost = "0.0.0.0";
    std::uint16_t remotePort = 1234;
    std::uint16_t localPort = 1234;
    std::size_t dataSize = 1400;
    std::vector<std::uint8_t> buf(dataSize, 0);
    sockaddr_in sockaddrServer, sockaddrClient;
    ifreq ifr;

    std::int32_t tunfd = -1, sockfd = -1;

    if(tunfd = open("/dev/net/tun", O_RDWR); tunfd < 0) {
        std::cerr << "failed open tun\n";
        return 1;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN;
    strncpy(ifr.ifr_name, tun.c_str(), IFNAMSIZ);

    if (auto err = ioctl(tunfd, TUNSETIFF, (void*)&ifr); err < 0) {
        std::cerr << "failed ioctl TUNSETIFF\n"; 
        close(tunfd);
        return 1;
    }

    if (sockfd = socket(AF_INET, SOCK_DGRAM, 0); sockfd < 0) {
        std::cerr << "failed open tun\n";
        close(tunfd);
        return 1;
    }

    sockaddrClient = sockaddr_in {
        .sin_family = AF_INET,
        .sin_port = htons(localPort),
        .sin_addr = {
            .s_addr = inet_addr(localHost.c_str())
        },
        .sin_zero = {0}
    };

    if (auto err = bind(sockfd, reinterpret_cast<const sockaddr*>(&sockaddrClient), sizeof(sockaddrClient)); err < 0) {
        std::cerr << "failed bind socket\n";
        close(tunfd);
        close(sockfd);
        return 1;
    }

    sockaddrServer = sockaddr_in {
        .sin_family = AF_INET,
        .sin_port = htons(remotePort),
        .sin_addr = {
            .s_addr = inet_addr(remoteHost.c_str())
        },
        .sin_zero = {0}
    };

    fcntl(sockfd, F_SETFL, O_NONBLOCK);
    fcntl(tunfd, F_SETFL, O_NONBLOCK);
    auto  maxfd = ( sockfd > tunfd) ? sockfd : tunfd;

    for(;;) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(sockfd, &rfds);
        FD_SET(tunfd, &rfds);

        auto ret = select(maxfd + 1, &rfds, NULL, NULL, NULL);

        if (ret < 0) {
            continue;
        }

        if (FD_ISSET(tunfd, &rfds)) {
            auto readSize = read(tunfd, buf.data(), dataSize);
            if (readSize <= 0) {
                continue;
            }
            std::cout << "read:" << readSize << std::endl;
            sendto(sockfd, buf.data(), readSize, 0, reinterpret_cast<const sockaddr*>(&sockaddrServer), sizeof(sockaddrServer));
        }

        if (FD_ISSET(sockfd, &rfds)) {
            auto readSize = recv(sockfd, buf.data(), dataSize, 0);
            if (readSize <= 0) {
                continue;
            }
            std::cout << "write" << readSize << std::endl;
            write(tunfd, buf.data(), readSize);
        }
    }
}
