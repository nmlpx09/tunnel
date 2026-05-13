#include "utils.h"

#include <fstream>

namespace NCrypt {

std::optional<std::pair<std::string, std::string>> loadKeyPair(const std::string& keysFile) {
    std::ifstream file(keysFile);
    std::string chiper;
    std::string iv;
    if (!file.is_open() || !std::getline(file, chiper) || !std::getline(file, iv)) {
        return {};
    }
    return std::make_pair(chiper, iv);
}

}
