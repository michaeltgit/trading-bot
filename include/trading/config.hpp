#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

namespace trading {

class Config {
public:
    Config() = default;
    ~Config() = default;

    bool loadFromFile(const std::string& path);
    std::string getString(std::string_view key) const;
    int getInt(std::string_view key) const;
    double getDouble(std::string_view key) const;

private:
    std::unordered_map<std::string, std::string> data_;
};

} // namespace trading