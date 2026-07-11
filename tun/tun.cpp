#include "tun.h"

#include <errors.h>

#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstring>

namespace NTun {

TTun::~TTun() {
    if (Fd >= 0) {
        close(Fd);
        Fd = -1;
    }
}

TTun::TTun(std::size_t maxBufferSize)
: MaxBufferSize(maxBufferSize)
, Buffer(MaxBufferSize, 0) { }

std::error_code TTun::Init(const std::string& deviceName) noexcept {
    if (Fd = open("/dev/net/tun", O_RDWR); Fd < 0) {
        return EErrorCode::TunOpen;
    }

    ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN;
    std::strncpy(ifr.ifr_name, deviceName.c_str(), IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    if (auto ret = ioctl(Fd, TUNSETIFF, &ifr); ret < 0) {
        return EErrorCode::TunBind;
    }

    if (auto ret = fcntl(Fd, F_SETFL, O_NONBLOCK); ret < 0) {
        return EErrorCode::TunConfig;
    }
    return {};
}

void TTun::Write(const TBuffer& buffer, std::size_t size) const noexcept {
    if (Fd < 0 || size == 0) {
        return;
    }

    const auto writeSize = write(Fd, buffer.data(), size);

    if (writeSize < 0) {
        return;
    }
}

std::tuple<
    std::reference_wrapper<const TBuffer>,
    std::size_t
> TTun::Read() noexcept {
    if (Fd < 0) {
        return {cref(Buffer), 0};
    }

    const auto readSize = read(Fd, Buffer.data(), MaxBufferSize);

    if (readSize <= 0) {
        return {cref(Buffer), 0};
    }

    return {cref(Buffer), readSize};
}

}
