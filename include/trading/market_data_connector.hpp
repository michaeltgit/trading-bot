#pragma once

#include <functional>
#include <memory>
#include <string>

#include "trading/limit_order_book.hpp"

namespace trading {

struct MarketDataUpdate {
    std::string symbol;
    double price;
    double size;
    bool isBid;
    std::string venue;
};

class MarketDataConnector {
public:
    using Callback = std::function<void(const MarketDataUpdate&)>;

    explicit MarketDataConnector(std::string symbol);
    ~MarketDataConnector();

    void connect();
    void disconnect();

    void subscribe(const std::string& symbol);
    void poll();

    void emit(const MarketDataUpdate& update);
    void setCallback(Callback cb);

    LimitOrderBook& getOrderBook();
    const std::string& getSymbol() const;

private:
    class BinanceWSClient;
    std::shared_ptr<BinanceWSClient> wsClient_;
    Callback cb_;
    std::string symbol_;
    LimitOrderBook orderBook_;
};

} // namespace trading