#include <cassert>
#include <iostream>

#include <spdlog/spdlog.h>

#include "trading/execution_engine.hpp"
#include "trading/limit_order_book.hpp"
#include "trading/execution_report.hpp"
#include "trading/new_order.hpp"

int main() {
    using namespace trading;

    ExecutionEngine engine;
    LimitOrderBook orderBook("TEST");

    orderBook.onUpdate(Side::Ask, 42.5, 100);
    engine.setOrderBook(&orderBook);

    bool callbackTriggered = false;
    ExecutionReport report{};

    engine.setCallback([&](const ExecutionReport &rpt) {
        callbackTriggered = true;
        report = rpt;
    });

    engine.connect("TEST");

    NewOrder order{"OID123", "BTCUSDT", true, 42.5, 100};
    std::string returnedId = engine.sendOrder(order);

    assert(callbackTriggered && "[FAIL] Execution callback was not triggered");
    assert(returnedId == "OID123" && "[FAIL] Returned orderId mismatch");
    assert(report.orderId == "OID123");
    assert(report.isFill && "[FAIL] Expected simulated fill");
    assert(report.execPrice == 42.5);
    assert(report.execSize == 100.0);

    callbackTriggered = false;
    bool cancelSuccess = engine.cancelOrder("OID123");

    assert(cancelSuccess && "[FAIL] Cancel failed");
    assert(callbackTriggered && "[FAIL] Cancel callback not triggered");
    assert(report.orderId == "OID123");
    assert(!report.isFill && "[FAIL] Cancel report should not be a fill");
    assert(report.execSize == 0.0);

    engine.disconnect();

    spdlog::info("[PASS] ExecutionEngine test succeeded.");
    return 0;
}