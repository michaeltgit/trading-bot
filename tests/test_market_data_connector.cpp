#include <cassert>
#include <iostream>
#include <string>
#include <functional>

#include <spdlog/spdlog.h>

#include "trading/market_data_connector.hpp"

class TestMarketDataConnector {
public:
    void setCallback(std::function<void(const trading::MarketDataUpdate&)> cb) {
        callback_ = std::move(cb);
    }

    void emit(const trading::MarketDataUpdate& update) {
        if (callback_) callback_(update);
    }

private:
    std::function<void(const trading::MarketDataUpdate&)> callback_;
};

int main() {
    using namespace trading;

    TestMarketDataConnector mdc;
    bool callbackInvoked = false;
    MarketDataUpdate seenUpdate;

    mdc.setCallback([&](const MarketDataUpdate& update) {
        callbackInvoked = true;
        seenUpdate = update;
    });

    MarketDataUpdate testUpdate{
        .symbol = "FAKE",
        .price = 100.0,
        .size = 1.0,
        .isBid = true
    };

    mdc.emit(testUpdate);

    assert(callbackInvoked && "[FAIL] Callback not triggered");
    assert(seenUpdate.symbol == "FAKE" && "[FAIL] Symbol mismatch");
    assert(seenUpdate.price == 100.0 && "[FAIL] Price mismatch");
    assert(seenUpdate.size == 1.0 && "[FAIL] Size mismatch");
    assert(seenUpdate.isBid && "[FAIL] isBid mismatch");

    spdlog::info("[PASS] MarketDataConnector logic test succeeded.");
    return 0;
}