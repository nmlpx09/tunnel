#pragma once

#include <memory>
#include <string>
#include <system_error>

namespace NLog {

struct TLog {
public:
    TLog();
    TLog(const TLog&) = delete;
    TLog(TLog&&) = delete;
    TLog& operator=(const TLog&) = delete;
    TLog& operator=(TLog&&) = delete;
    ~TLog();

    void LogInfo(const std::string& message) const noexcept;
    void LogError(const std::string& message) const noexcept;
    void LogErrorCode(const std::error_code& ec) const noexcept;
};

using TLogPtr = std::shared_ptr<const TLog>;

}
