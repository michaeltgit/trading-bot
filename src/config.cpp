#include "trading/config.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace trading {

bool Config::loadFromFile(const std::string& path) {
    std::ifstream in{path};
    if (!in.is_open()) {
        return false;
    }

    data_.clear();
    std::string line;
    while (std::getline(in, line)) {
        auto pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }
        auto key = line.substr(0, pos);
        auto val = line.substr(pos + 1);
        data_[key] = val;
    }
    return true;
}

std::string Config::getString(std::string_view key) const {
    auto it = data_.find(std::string(key));
    if (it == data_.end()) {
        throw std::runtime_error("Missing config key: " + std::string(key));
    }
    return it->second;
}

int Config::getInt(std::string_view key) const {
    return std::stoi(getString(key));
}

double Config::getDouble(std::string_view key) const {
    return std::stod(getString(key));
}

} // namespace trading