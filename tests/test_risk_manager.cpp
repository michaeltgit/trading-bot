#include <cassert>
#include <iostream>

#include <spdlog/spdlog.h>

#include "trading/execution_engine.hpp"
#include "trading/risk_manager.hpp"

int main() {
    using namespace trading;

    bool wasRejected = false;
    std::string lastRejectedId;

    RiskManager rm(100);

    rm.setRejectCallback([&](const NewOrder& o) {
        wasRejected = true;
        lastRejectedId = o.orderId;
    });

    NewOrder o1{"O1", "TEST", true, 10.0, 50};
    assert(rm.approveOrder(o1) && "Expected approval for O1");

    wasRejected = false;
    lastRejectedId.clear();
    NewOrder o2{"O2", "TEST", true, 10.0, 200};
    assert(!rm.approveOrder(o2) && "Expected rejection for O2");
    assert(wasRejected && lastRejectedId == "O2");

    ExecutionReport fill1{"F1", "TEST", true, true, 10.0, 50};
    rm.onExecutionReport(fill1);
    assert(rm.getPosition("TEST") == 50.0);

    wasRejected = false;
    lastRejectedId.clear();
    NewOrder o3{"O3", "TEST", true, 10.0, 60};
    assert(!rm.approveOrder(o3) && "Expected rejection pushing to 110");
    assert(wasRejected && lastRejectedId == "O3");

    NewOrder o4{"O4", "TEST", false, 10.0, 40};
    assert(rm.approveOrder(o4) && "Expected approval for O4 (sell)");

    ExecutionReport fill2{"F2", "TEST", false, true, 10.0, 40};
    rm.onExecutionReport(fill2);
    assert(rm.getPosition("TEST") == 10.0);

    wasRejected = false;
    lastRejectedId.clear();
    NewOrder o5{"O5", "TEST", false, 10.0, 120};
    assert(!rm.approveOrder(o5) && "Expected rejection for short beyond limit");
    assert(wasRejected && lastRejectedId == "O5");

    spdlog::info("[PASS] RiskManager test succeeded.");
    return 0;
}