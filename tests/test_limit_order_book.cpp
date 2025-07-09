#include <cassert>
#include <iostream>

#include <spdlog/spdlog.h>

#include "trading/limit_order_book.hpp"

int main() {
    using namespace trading;

    LimitOrderBook lob("TEST");

    try {
        lob.topOfBook(Side::Bid);
        assert(false && "[FAIL] Expected exception on empty bid book");
    } catch (...) {}

    try {
        lob.topOfBook(Side::Ask);
        assert(false && "[FAIL] Expected exception on empty ask book");
    } catch (...) {}

    BookSnapshot snap;
    snap.bids.push_back({100.0, 10.0});
    snap.asks.push_back({101.0, 5.0});
    lob.reset(snap);

    auto bestBid = lob.topOfBook(Side::Bid);
    assert(bestBid.price == 100.0 && "[FAIL] Bid price mismatch");
    assert(bestBid.size == 10.0 && "[FAIL] Bid size mismatch");

    auto bestAsk = lob.topOfBook(Side::Ask);
    assert(bestAsk.price == 101.0 && "[FAIL] Ask price mismatch");
    assert(bestAsk.size == 5.0 && "[FAIL] Ask size mismatch");

    lob.onUpdate(Side::Bid, 100.0, 8.0);
    bestBid = lob.topOfBook(Side::Bid);
    assert(bestBid.size == 8.0 && "[FAIL] Updated bid size mismatch");

    lob.onUpdate(Side::Ask, 101.0, 0.0);
    try {
        lob.topOfBook(Side::Ask);
        assert(false && "[FAIL] Expected exception after removing last ask");
    } catch (...) {}

    lob.onUpdate(Side::Ask, 102.0, 2.0);
    lob.onUpdate(Side::Ask, 103.0, 4.0);
    auto asks = lob.depth(Side::Ask, 5);
    assert(asks.size() == 2 && "[FAIL] Ask depth should return 2 levels");
    assert(asks[0].price == 102.0 && "[FAIL] First ask level price mismatch");
    assert(asks[1].price == 103.0 && "[FAIL] Second ask level price mismatch");

    spdlog::info("[PASS] LimitOrderBook test succeeded.");
    return 0;
}