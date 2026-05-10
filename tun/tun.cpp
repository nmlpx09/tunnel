#include "tun.h"

#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstring>

namespace NTun {

TTun::~TTun() {
    close(Fd);
    Fd = -1;
}

TTun::TTun(std::size_t maxBufferSize) noexcept
: MaxBufferSize(maxBufferSize)
, Buffer(MaxBufferSize, 0) { }

std::int32_t TTun::Init(std::string deviceName) noexcept {
    if(Fd = open("/dev/net/tun", O_RDWR); Fd < 0) {
        return -1;
    }

    ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN;
    std::strncpy(ifr.ifr_name, deviceName.c_str(), deviceName.size());

    if (auto ret = ioctl(Fd, TUNSETIFF, &ifr); ret < 0) {
        return -1;
    }

    if (auto ret = fcntl(Fd, F_SETFL, O_NONBLOCK); ret < 0){
        return -1;
    }
    return 0;
}

void TTun::Write(const TBuffer& buffer, std::size_t size) const noexcept {
    if (Fd < 0) {
        return;
    }

    if (size == 0) {
        return;
    }
    write(Fd, buffer.data(), size);
}

std::size_t TTun::Read() noexcept {
    if (Fd < 0) {
        return 0;
    }

    const auto readSize = read(Fd, Buffer.data(), MaxBufferSize);

    if (readSize <= 0) {
        return 0;
    }

    return readSize;
}

const TBuffer& TTun::GetBuffer() const noexcept {
    return Buffer;
}

bool TTun::IsFd(std::int32_t fd) const noexcept {
    return Fd == fd;
}

}
