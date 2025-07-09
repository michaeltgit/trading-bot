#include <cassert>
#include <iostream>
#include <vector>

#include <spdlog/spdlog.h>

#include "trading/telemetry.hpp"

int main() {
    using namespace trading;

    std::vector<std::pair<std::string, double>> seen;

    Telemetry::setPublisher([&](const std::string& key, double value) {
        seen.emplace_back(key, value);
    });

    Telemetry::increment("ctr");
    Telemetry::increment("ctr", 2);
    Telemetry::gauge("g1", 3.14);
    Telemetry::timing("t1", 5.0);
    Telemetry::timing("t1", 2.0);

    Telemetry::publishAll();

    bool gotCtr = false, gotG1 = false, gotT1 = false;

    for (const auto& [key, value] : seen) {
        if (key == "ctr.count" && value == 3.0)
            gotCtr = true;
        if (key == "g1.gauge" && value == 3.14)
            gotG1 = true;
        if (key == "t1.timing" && value == 7.0)
            gotT1 = true;
    }

    assert(gotCtr && "Missing or incorrect ctr.count");
    assert(gotG1 && "Missing or incorrect g1.gauge");
    assert(gotT1 && "Missing or incorrect t1.timing");

    spdlog::info("[PASS] Telemetry test succeeded.");
    return 0;
}