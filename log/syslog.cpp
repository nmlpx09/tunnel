#include "syslog.h"

#include <syslog.h>

#include <format>

namespace NLog {

TLog::TLog() {
    openlog("tun", LOG_NDELAY, LOG_DAEMON);
}

TLog::~TLog() {
    closelog();
}

void TLog::LogInfo(const std::string& message) const noexcept {
    syslog(LOG_NOTICE, "%s", message.c_str());
}

void TLog::LogError(const std::string& message) const noexcept {
    syslog(LOG_ERR, "error: %s", message.c_str());
}

void TLog::LogErrorCode(const std::error_code& ec) const noexcept {
    syslog(LOG_ERR, "error: %s",  ec.message().c_str());
}

}
