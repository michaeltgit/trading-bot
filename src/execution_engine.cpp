#include "trading/execution_engine.hpp"

#include "trading/limit_order_book.hpp"
#include "trading/telemetry.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <iomanip>
#include <sstream>
#include <utility>

namespace trading {

ExecutionEngine::ExecutionEngine()
    : logger_(spdlog::get("ExecutionEngine") ? spdlog::get("ExecutionEngine")
                                             : spdlog::stdout_color_mt("ExecutionEngine")) {}

ExecutionEngine::~ExecutionEngine() {
    disconnect();
}

void ExecutionEngine::connect(const std::string&) {
    logger_->info("Execution engine connected");
}

void ExecutionEngine::disconnect() {
    logger_->info("Execution engine disconnected");
}

void ExecutionEngine::setOrderBook(LimitOrderBook* orderBook) {
    std::lock_guard lk(mutex_);
    orderBook_ = orderBook;
}

std::string ExecutionEngine::sendOrder(const NewOrder& o) {
    std::lock_guard lk(mutex_);
    orderStates_[o.orderId] = OrderState::PendingNew;
    Telemetry::increment("orders.sent");

    logger_->info("[SIMULATED] Placing order {}: {} {:.4f} @ {:.2f}",
                  o.orderId, o.isBuy ? "BUY" : "SELL", o.size, o.price);

    simulateFill(o);
    return o.orderId;
}

void ExecutionEngine::simulateFill(const NewOrder& o) {
    double remaining = o.size;
    double filled = 0.0;
    double cost = 0.0;

    if (orderBook_) {
        auto side = o.isBuy ? Side::Ask : Side::Bid;
        auto book_side = orderBook_->depth(side, 20);

        for (const auto& lvl : book_side) {
            bool match = o.isBuy ? (lvl.price <= o.price) : (lvl.price >= o.price);
            if (!match) break;

            double fill_size = std::min(remaining, lvl.size);
            remaining -= fill_size;
            filled += fill_size;
            cost += fill_size * lvl.price;

            logger_->info("[SIMULATED] Level filled: {:.4f} @ {:.2f}", fill_size, lvl.price);

            if (remaining <= 1e-8) break;
        }

        if (std::abs(o.size - filled) > 1e-8) {
            logger_->warn("[SIMULATED] Order {} only partially filled: requested {:.4f}, filled {:.4f}",
                          o.orderId, o.size, filled);
        }
    }

    double avg_price = (filled > 0.0) ? (cost / filled) : 0.0;

    ExecutionReport rpt{
        o.orderId,
        o.symbol,
        o.isBuy,
        filled > 0.0,
        avg_price,
        filled
    };

    orderStates_[o.orderId] = OrderState::Filled;
    Telemetry::increment("orders.filled");

    logger_->info("[SIMULATED] Order {} fill complete: {} {:.4f} @ {:.2f} (average price)",
                  rpt.orderId, rpt.isBuy ? "BUY" : "SELL", rpt.execSize, rpt.execPrice);

    if (cb_) cb_(rpt);
}

bool ExecutionEngine::cancelOrder(const std::string& orderId) {
    std::lock_guard lk(mutex_);
    orderStates_[orderId] = OrderState::Canceled;
    Telemetry::increment("orders.cancelled");
    Telemetry::increment("orders.canceled_ack");

    logger_->info("[SIMULATED] Cancel for order {}", orderId);

    if (cb_) {
        ExecutionReport rpt{orderId, "", false, false, 0.0, 0.0};
        cb_(rpt);
    }

    return true;
}

void ExecutionEngine::setCallback(Callback cb) {
    std::lock_guard lk(mutex_);
    cb_ = std::move(cb);
}

} // namespace trading