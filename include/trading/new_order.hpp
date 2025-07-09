#pragma once

#include <string>
#include <utility>

namespace trading {

struct NewOrder {
    std::string orderId;
    std::string symbol;
    bool isBuy;
    double price;
    double size;

    NewOrder(std::string orderId,
             std::string symbol,
             bool isBuy,
             double price,
             double size)
        : orderId(std::move(orderId)),
          symbol(std::move(symbol)),
          isBuy(isBuy),
          price(price),
          size(size) {}
};

} // namespace trading