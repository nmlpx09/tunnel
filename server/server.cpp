#include <configs.h>
#include <crypt/crypt.h>
#include <ips_storage/ips_storage.h>
#include <socket/socket.h>
#include <log/syslog.h>
#include <poll/poll.h>
#include <tun/tun.h>
#include <utils/utils.h>

#include <atomic>
#include <csignal>
#include <memory>
#include <thread>

namespace {
    std::atomic<std::sig_atomic_t> signalStatus = 0;
}

void SignalHandler(int signal) {
    signalStatus.store(signal, std::memory_order_relaxed);
}

void tx(
    NLog::TLogPtr log,
    NPoll::TPollPtr poll,
    NTun::TTunPtr tun,
    NSocket::TSocketPtr socket,
    NCrypt::TCryptPtr crypt,
    NIpsStorage::TIpsStoragePtr ipsStorage
) noexcept {
    while(!signalStatus.load(std::memory_order_relaxed)) {
        const auto result = poll->Wait();
        if (!result) {
            log->LogError("tun poll exit");
            break;
        } else if(!result.value()) {
            continue;
        }
        while(!signalStatus.load(std::memory_order_relaxed)) {
            const auto buffer = tun->Read();
            if (buffer.empty()) {
                break;
            } else if (!NUtils::ValidIpv4Packet(buffer)) {
                continue;
            }
            if (const auto value = ipsStorage->Get(NUtils::GetDstIpFromIpv4Packet(buffer)); value) {
                const auto encrBuffer = crypt->Encrypt(buffer);
                socket->Write(encrBuffer, value->first, value->second);
            }
        }
    }
}

void rx(
    NLog::TLogPtr log,
    NPoll::TPollPtr poll,
    NTun::TTunPtr tun,
    NSocket::TSocketPtr socket,
    NCrypt::TCryptPtr crypt,
    NIpsStorage::TIpsStoragePtr ipsStorage
) noexcept {
    while(!signalStatus.load(std::memory_order_relaxed)) {
        const auto result = poll->Wait();
        if (!result) {
            log->LogError("socket poll exit");
            break;
        } else if(!result.value()) {
            continue;
        }
        while (!signalStatus.load(std::memory_order_relaxed)) {
            const auto [buffer, ip, port] = socket->Read();
            if (buffer.empty()) {
                break;
            }
            const auto decrBuffer = crypt->Decrypt(buffer);
            if (!NUtils::ValidIpv4Packet(decrBuffer)) {
                continue;
            }

            ipsStorage->Add(NUtils::GetSrcIpFromIpv4Packet(decrBuffer), ip, port);

            tun->Write(decrBuffer);
        }
    }
}

int main() {
    const auto log = std::make_shared<const NLog::TLog>();
    try {
        const auto& [ec, conf] = NUtils::GetConf(false);

        if (ec) {
            log->LogErrorCode(ec);
            return 1;
        }

        const auto tun = std::make_shared<NTun::TTun>(MAX_DATA_SIZE);

        if (auto ec = tun->Init(conf.TunDevice); ec) {
            log->LogErrorCode(ec);
            return 2;
        }

        const auto socket = std::make_shared<NSocket::TSocket>(MAX_DATA_SIZE);

        if (auto ec = socket->Init(conf.LocalIp, conf.LocalPort); ec) {
            log->LogErrorCode(ec);
            return 3;
        }

        const auto pollTun = std::make_shared<NPoll::TPoll>(MAX_POLL_TIMEOUT_MS);

        if (auto ec = pollTun->Init(); ec) {
            log->LogErrorCode(ec);
            return 4;
        }

        if (auto ec = pollTun->RegisterFd(tun->GetFd()); ec) {
            log->LogErrorCode(ec);
            return 5;
        }

        const auto pollSocket = std::make_shared<NPoll::TPoll>(MAX_POLL_TIMEOUT_MS);

        if (auto ec = pollSocket->Init(); ec) {
            log->LogErrorCode(ec);
            return 6;
        }

        if (auto ec = pollSocket->RegisterFd(socket->GetFd()); ec) {
            log->LogErrorCode(ec);
            return 7;
        }

        const auto ipsStorage = std::make_shared<NIpsStorage::TIpsStorage>();

        const auto crypt = std::make_shared<NCrypt::TCrypt>(MAX_DATA_SIZE);

        const auto key = NUtils::LoadKey(conf.KeysFile);

        if (!key) {
            log->LogError("failed load key");
            return 8;
        }

        if (auto ec = crypt->Init(key.value()); ec) {
            log->LogErrorCode(ec);
            return 9;
        }

        signal(SIGINT, SignalHandler);

        std::thread tTx(tx, log, pollTun, tun, socket, crypt, ipsStorage);
        std::thread tRx(rx, log, pollSocket, tun, socket, crypt, ipsStorage);

        log->LogInfo("run server");

        tTx.join();
        tRx.join();

        log->LogInfo("stop server");
    } catch (const std::exception& exc) {
        log->LogError(exc.what());
        return 10;
    }
}
