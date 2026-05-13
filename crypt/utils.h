#pragma once

#include <optional>
#include <string>
#include <utility>

namespace NCrypt {

std::optional<std::pair<std::string, std::string>> loadKeyPair(const std::string& keysFile);

}
