#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include <spdlog/spdlog.h>

#include "trading/execution_report.hpp"
#include "trading/limit_order_book.hpp"
#include "trading/new_order.hpp"
#include "trading/order_state.hpp"

namespace trading {

class ExecutionEngine {
public:
    using Callback = std::function<void(const ExecutionReport&)>;

    ExecutionEngine();
    ~ExecutionEngine();

    void connect(const std::string& endpoint);
    void disconnect();

    std::string sendOrder(const NewOrder& order);
    bool cancelOrder(const std::string& orderId);

    void setCallback(Callback cb);
    void setOrderBook(LimitOrderBook* orderBook);

private:
    void simulateFill(const NewOrder& order);

    std::mutex mutex_;
    Callback cb_;
    std::unordered_map<std::string, OrderState> orderStates_;
    std::shared_ptr<spdlog::logger> logger_;
    LimitOrderBook* orderBook_ = nullptr;
};

} // namespace trading