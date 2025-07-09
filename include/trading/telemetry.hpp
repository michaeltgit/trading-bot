#pragma once

#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>

namespace trading {

class Telemetry {
public:
    using PublishFn = std::function<void(const std::string& key, double value)>;

    static void setPublisher(PublishFn pub);
    static void increment(const std::string& key, int64_t n = 1);
    static void gauge(const std::string& key, double value);
    static void timing(const std::string& key, double ms);
    static void publishAll();

private:
    struct Bucket {
        double lastGauge = 0.0;
        int64_t count = 0;
        double sumTimings = 0.0;
    };

    static std::unordered_map<std::string, Bucket> metrics_;
    static std::mutex mtx_;
    static PublishFn publisher_;
};

} // namespace trading