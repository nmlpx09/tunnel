#include "tun.h"

#include <errors.h>

#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstring>
#include <utility>

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

std::error_code TTun::Init(const std::string& deviceName) {
    if (Fd >= 0) {
        return {};
    }
 
    if (Fd = open("/dev/net/tun", O_RDWR); Fd < 0) {
        return EErrorCode::TunOpen;
    }

    ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    std::strncpy(ifr.ifr_name, deviceName.c_str(), IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    if (auto ret = ioctl(Fd, TUNSETIFF, &ifr); ret < 0) {
        return EErrorCode::TunBind;
    }

    if (auto flags = fcntl(Fd, F_GETFL, 0); flags < 0) {
        return EErrorCode::TunConfig;
    } else {
        if (auto ret = fcntl(Fd, F_SETFL, flags | O_NONBLOCK); ret < 0) {
            return EErrorCode::TunConfig;
        }
    }

    return {};
}

std::int32_t TTun::GetFd() const {
    return Fd;
}

std::error_code TTun::Write(TBufferView buffer) const noexcept {
    if (Fd < 0) {
        return EErrorCode::TunOpen;
    }

    if (buffer.empty()) {
        return {};
    }

    const auto writeSize = write(Fd, buffer.data(), buffer.size());

    if (writeSize < 0 || static_cast<std::size_t>(writeSize) != buffer.size()) {
        return EErrorCode::TunWrite;
    }

    return {};
}

std::expected<TBufferView, std::error_code> TTun::Read() noexcept {
    if (Fd < 0) {
        return std::unexpected(EErrorCode::TunOpen);
    }

    const auto readSize = read(Fd, Buffer.data(), MaxBufferSize);

    if (readSize < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return {};
        }
        return std::unexpected(EErrorCode::TunRead);
    }

    if (readSize == 0) {
        return {};
    }

    return TBufferView{Buffer.begin(), static_cast<std::size_t>(readSize)};
}

}
