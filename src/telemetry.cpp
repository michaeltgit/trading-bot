#include "trading/telemetry.hpp"
#include <spdlog/spdlog.h>

#include <string>
#include <unordered_map>
#include <mutex>
#include <utility>

namespace trading {

std::unordered_map<std::string, Telemetry::Bucket> Telemetry::metrics_;
std::mutex Telemetry::mtx_;
Telemetry::PublishFn Telemetry::publisher_ = [](const std::string& k, double v) {
    spdlog::info("{}: {}", k, v);
};

void Telemetry::setPublisher(PublishFn pub) {
    std::lock_guard lock(mtx_);
    publisher_ = std::move(pub);
}

void Telemetry::increment(const std::string& key, int64_t n) {
    std::lock_guard lock(mtx_);
    metrics_[key].count += n;
}

void Telemetry::gauge(const std::string& key, double value) {
    std::lock_guard lock(mtx_);
    metrics_[key].lastGauge = value;
}

void Telemetry::timing(const std::string& key, double ms) {
    std::lock_guard lock(mtx_);
    metrics_[key].sumTimings += ms;
}

void Telemetry::publishAll() {
    std::lock_guard lock(mtx_);
    for (const auto& [k, b] : metrics_) {
        publisher_(k + ".count", b.count);
        publisher_(k + ".gauge", b.lastGauge);
        publisher_(k + ".timing", b.sumTimings);
    }
}

} // namespace trading