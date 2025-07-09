#pragma once

#include <map>
#include <string>
#include <vector>

namespace trading {

enum class Side {
    Bid,
    Ask
};

struct OrderBookEntry {
    double price;
    double size;
};

struct BookSnapshot {
    std::vector<OrderBookEntry> bids;
    std::vector<OrderBookEntry> asks;
};

class LimitOrderBook {
public:
    explicit LimitOrderBook(const std::string& symbol);

    void reset(const BookSnapshot& snapshot);
    void resetTopLevels(const std::vector<OrderBookEntry>& bids, const std::vector<OrderBookEntry>& asks);
    void onUpdate(Side side, double price, double size);

    OrderBookEntry topOfBook(Side side) const;
    std::vector<OrderBookEntry> depth(Side side, size_t levels) const;

    void handleMarketDataUpdate(double price, double size, bool isBid);

private:
    std::string symbol_;
    std::map<double, double> bids_;
    std::map<double, double> asks_;
};

} // namespace trading